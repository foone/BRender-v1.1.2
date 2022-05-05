/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: zbtest.c 1.82 1995/08/31 16:48:05 sam Exp $
 * $Locker:  $
 *
 * $BC<"make -f zbtest.mak %s.obj;">
 *
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <conio.h>

#include "brender.h"
#include "dosio.h"
#include "syshost.h"

#include "brassert.h"
#include "zb.h"
#include "fmt.h"
#include "blockops.h"
#include "zbproto.h"
#include "datafile.h"
#include "brassert.h"

#include "trigen.h"
#include "resdbg.h"

#include "trackmem.h"

#if HAS_ENV_H
#include <env.h>
#endif

static char rscid[] = "$Id: zbtest.c 1.82 1995/08/31 16:48:05 sam Exp $";

#if BASED_FIXED
#define SUFFIX_BASED "-FIXED"
#endif

#if BASED_FLOAT
#define SUFFIX_BASED "-FLOAT"
#endif

#define USE_KBD 1
#define USE_DIVTRAP 0

#define FLIP_Y 0

#define TIMING_FRAMES 61
#define USE_SYSTEM_CLOCK 0

/*
 * Either use system clock, or our own
 */
#if USE_SYSTEM_CLOCK
#include <time.h>

#define CLOCKREAD() clock()
#define CLOCKINIT() 
#undef CLOCK_RATE
#define CLOCK_RATE CLOCKS_PER_SEC

#else

#define CLOCK_RATE BR_DOS_CLOCK_RATE

#define CLOCKREAD() DOSClockRead()
#define CLOCKINIT() DOSClockBegin()
#define CLOCKTERM() DOSClockEnd()

#endif

#define X 0
#define Y 1
#define Z 2
#define W 3

/*
 * Pixelmap pointers
 */
br_pixelmap *screen_buffer;
br_pixelmap *depth_buffer;
br_pixelmap *back_buffer;

br_camera camera_data = {
	"main_camera",
	BR_CAMERA_PERSPECTIVE,
//	BR_CAMERA_PARALLEL,
	BR_ANGLE_DEG(45),	/* 45 degree field of view	*/
	BR_SCALAR(0.1),		/* Near clip plane 			*/
	BR_SCALAR(30.0),	/* Far clip plane			*/
	BR_SCALAR(1.46),	/* aspect ratio				*/
	BR_SCALAR(1.5),
	BR_SCALAR(1.5),
	BR_SCALAR(1.0),
};

br_light light_data = {
	"main_light",
	BR_LIGHT_DIRECT, 					/* type */
	BR_COLOUR_RGB(255,255,255),			/* colour */
	BR_SCALAR(1.0),						/* attenuation_c */
	BR_SCALAR(0.0),						/* attenuation_l */
	BR_SCALAR(0.0),						/* attenuation_q */
	BR_ANGLE_DEG(15.0),					/* cone_outer */
	BR_ANGLE_DEG(10.0),					/* cone_inner */
};

/*
 * Some useful materials
 */
br_material *pick_material;
br_material *axis_material;
br_material *axis_material_os;
br_material *animate_material;

br_matrix34 Rotation,ObsRotation,LightRotation,ClipRotation,RotationX,RotationY,RotationZ;

br_matrix34 DebugMatrix1,DebugMatrix2;
br_matrix4 DebugMatrix3,DebugMatrix4,DebugMatrix5;
int DebugCount0;
int DebugCount1;

br_scalar pos_x=0,pos_y=0,pos_z=BR_SCALAR(0.0);
br_scalar obs_x=0,obs_y=0,obs_z=BR_SCALAR(6.0);
br_scalar light_x=0,light_y=0,light_z=BR_SCALAR(2.0);
int ModelScale = 16;

int PickCursorOn = 0;
int PickCursor_X = 0;
int PickCursor_Y = 0;
int PickCount = 0;

typedef struct br_pick_nearest {
	br_actor *actor;
	br_model *model;
	br_material *material;
	br_vector3 point;
	int face;
	int edge;
	int vertex;
	br_vector2 map;

	br_scalar t;
	br_actor *temp_actor;
} br_pick_nearest;

br_pick_nearest PickNearest;

int RotateObjs = 1;
int Timing = 0;
int ToggleFlag = 0;
int DebugFlag = 1;
br_angle counter;

br_model *morph_models[32];
int nmorph_models;

void dprintf(int x, int y, char *fmt,...);

void DebugInfoMatrices(void);
void DebugInfoDebugMatrices(void);
void DebugInfoVertices(void);
void DebugInfoGroups(void);
void DebugInfoMisc(void);
void DebugInfoClipPlanes(void);
void DebugInfoRenderIdents(void);
void DebugInfoZConvert(void);
void DebugInfoKeyMatrix(void);

void PickDisplay(void);

void InteractiveLoop(void);
void InteractiveTimingLoop(void);
void CacheTimingLoop(void);

void AddFourChildren(br_actor *a, int level);
void GridActors(br_actor * parent,
	int nx, int ny, int nz,
	br_scalar sx, br_scalar sy, br_scalar sz,
	br_scalar dx, br_scalar dy, br_scalar dz, int t);

void BR_CALLBACK TestBoundsCallback(
	br_actor *actor,
	br_model *model,
	br_material * material,
	br_uint_8	style,
	br_matrix4 *model_to_screen,
	br_int_32 *bounds);
void BR_CALLBACK GetM2SCallback(
	br_actor *actor,
	br_model *model,
	br_material *material,
	br_uint_8	style,
	br_matrix4 *model_to_screen,
	br_int_32 *bounds);

void LabelModelCallback(
				br_actor *actor,
				br_model *model,
				br_material *material,
				br_uint_8 style,
				int on_screen,
				br_matrix34 *model_to_view,
				br_matrix4 *model_to_screen);

void FlushBounds(void);
void MorphModels(br_model *dest, br_model *model_a, br_model *model_b, br_scalar alpha);
void MorphModelsMany(br_model *dest, br_model **models, int nmodels, br_scalar alpha);

br_pixelmap * BR_CALLBACK LoadTableFFHook(char *name);
br_pixelmap * BR_CALLBACK LoadMapFFHook(char *name);

void PrelightTest(void);
void Pick3DTest(void);

/*
 * Test shapes to load up
 */
struct {
	char *name;
	char *material;
	br_model *model;
} TestModels[] = {

#if 0
	{"cube.dat",	"DEFAULT",},
#endif

#if 1
	{"cube.dat",	"DEFAULT",},
	{"tri.txt",		"DEFAULT",},
	{"square.txt",	"DEFAULT",},
	{"cube2.dat",	"DEFAULT",},
	{"torus.dat",	"DEFAULT",},
	{"torus.dat",	NULL, },
	{"axis1.dat",	"DEFAULT",},
#endif

#if 0
	{"teapot.dat","DEFAULT",},
	{"envtpot.dat",	NULL, },
	{"venus.dat",	"DEFAULT",},
	{"galleon.dat",	NULL,},
	{"aircar.dat",NULL,},
	{"terrain5.dat",NULL,},
	{"terrain3.dat",NULL,},
	{"teapot4k.dat",NULL,},
	{"torus.dat",	NULL,},

	{"torus.dat",	"DEFAULT",},
	{"sph8.dat",	"DEFAULT",},
	{"sph16.dat",	"DEFAULT",},
	{"sph32.dat",	"DEFAULT",},
	{"cube.dat",	"DEFAULT",},
	{"cube2.dat",	"DEFAULT",},
	{"cube4.dat",	"DEFAULT",},
	{"cyl2.dat",	"DEFAULT",},
	{"cylinder.dat","DEFAULT",},
	{"axis1.dat",	"DEFAULT",},
#endif
};

int CurrentModel = 0;

br_model *light_model,*axis_model,*cube_model;
br_actor *observer,*light,*light_child,*test_world,*test_shape,*test_shape_q,*test_shape_l1,*test_shape_l2;
br_actor *bounds_actor;
br_bounds test_bounds;

#define SHOW(expr,format) printf(#expr " = " format "\n",expr)

int max_debug_lines = 31;

br_actor *robot;
br_actor *robot_bit;

int main(int argc, char **argv)
{
	br_actor *a,*a1,*a2,*a3;
	br_pixelmap *pm,*pma[10];
	br_pixelmap *st;
	br_material *mat,*mata[10];
	br_material *mats[128];
	int n,i,j;
	BR_BANNER("ZBTEST" BR_SUFFIX_HOST BR_SUFFIX_DEBUG,"1992-1995","$Revision: 1.82 $");

	/*
	 * Set the search path
	 */
#if HAS_SETENV
	if(getenv("BRENDER_PATH") == NULL)
		setenv("BRENDER_PATH","../dat;../maps;dat;maps",1);
#endif

	BrBegin();

#if 0
	{
		struct block {
			int i;
			int j;
			int k;
			int l;
		};
		br_pool *pool;
		struct block *test[2048];
		int shuffle[2048];
		int i,a,b,t;

		for(i=0; i < BR_ASIZE(shuffle); i++)
			shuffle[i] = i;

		srand(102);
		for(i=0; i < BR_ASIZE(shuffle); i++) {
			a = rand() % BR_ASIZE(shuffle);
			b = rand() % BR_ASIZE(shuffle);
			t = shuffle[a];
			shuffle[a] = shuffle[b];
			shuffle[b] = t;
		}
		pool = BrPoolAllocate(sizeof(struct block), 45, BR_MEMORY_APPLICATION);

		for(i=0; i < BR_ASIZE(test); i++) {
			test[i] = BrPoolBlockAllocate(pool);
			test[i]->i = i;
			test[i]->j = i;
			test[i]->k = i;
			test[i]->l = i;
		}		

		for(i=0; i < BR_ASIZE(test); i++) {
			a = shuffle[i];

			if(test[a] == NULL)
				BR_ERROR0("Pool FU 1");

			if((test[a]->i != a) ||
			   (test[a]->j != a) ||
			   (test[a]->k != a) ||
			   (test[a]->l != a))
				BR_ERROR0("Pool FU 2");

			BrPoolBlockFree(pool, test[a]);

			test[a] = NULL;
		}		

		for(i=0; i < BR_ASIZE(test); i++)
			if(test[a] != NULL)
				BR_ERROR0("Pool FU 3");

		BrPoolFree(pool);

		exit(0);
	}


#endif

#if 0
	{
		br_quat q1,q2,q,qn;
	 	float f,l;

		q1.x = BR_SCALAR(1.0);
		q1.y = BR_SCALAR(0.0);
		q1.z = BR_SCALAR(0.0);
		q1.w = BR_SCALAR(0.5);

		q2.x = BR_SCALAR(0.0);
		q2.y = BR_SCALAR(1.0);
		q2.z = BR_SCALAR(0.0);
		q2.w = BR_SCALAR(0.5);

		BrQuatNormalise(&q1,&q1);
		BrQuatNormalise(&q2,&q2);

		for(f=0.0; f < 1.0F; f += 0.1) {
			
			BrQuatSlerp(&q,&q1,&q2,BrFloatToScalar(f),0);
			BrQuatNormalise(&qn,&q);
			l = BrScalarToFloat(BR_LENGTH4(q.x,q.y,q.z,q.w));

			printf("%5f: q (%f,%f,%f,%f) l %f qn (%f,%f,%f,%f)\n",
				f,
				BrScalarToFloat(q.x),
				BrScalarToFloat(q.y),
				BrScalarToFloat(q.z),
				BrScalarToFloat(q.w),
				l,
				BrScalarToFloat(qn.x),
				BrScalarToFloat(qn.y),
				BrScalarToFloat(qn.z),
				BrScalarToFloat(qn.w));
		}
	
		exit(10);
	}

#endif

#if 0
	/*
	 * Test Cos/ACos
	 */
	for(i = 0; i < 65536; i+= 150) {
		br_scalar s,c;
		double a;

		a = BrScalarToFloat(BrAngleToRadian(i));

		c = BR_COS(i);
		s = BR_SIN(i);

		printf("%04x,%04x %+9.7f:",i,BrRadianToAngle(BrFloatToScalar(a)), a);
		printf("C:%+9.7f,%+9.7f ",	BrScalarToFloat(c),			cos(a));
		printf("AC:%04x,%04x ",		BR_ACOS(c),					BrRadianToAngle(BrFloatToScalar(acos(BrScalarToFloat(c)))));
		printf("S:%+9.7f,%+9.7f ",	BrScalarToFloat(s),			sin(a));
		printf("AS:%04x,%04x\n",	BR_ASIN(s),					BrRadianToAngle(BrFloatToScalar(asin(BrScalarToFloat(s)))));
	}

	exit(0);
#endif

#if 0
	{
		dosio_event e;
		br_int_32 x,y;
		br_uint_32 buttons = 0;

		DOSEventBegin();
		DOSKeyBegin();
		DOSMouseBegin();

		DOSKeyEnableBIOS(1);

		do {
#if 1
			DOSEventWait(&e,1);

			printf("%08lx %08lx %08lx %08lx\n",
				e.type,
				e.qualifiers,
				e.value_1,
				e.value_2);
#endif

			/*
			 * Suck any BIOS keys
			 */
			while(KBHIT())
				printf("BIOS key: %02x\n",GETKEY());

			DOSMouseRead(&x,&y,&buttons);

		} while (!(buttons & BR_MSM_BUTTONR));

		DOSMouseEnd();
		DOSKeyEnd();
		DOSEventEnd();

		exit(10);
	}
#endif

#if 0
	{
		br_model *model;
		int n;

		n = BrFmtASCLoad("p:/tech/q3d/meshes/blocka.asc",&model,1);

		if(n == 0)
			BR_ERROR0("Could not load model");

		if(model->identifier)
			SHOW(model->identifier,"%s");

		SHOW(model->nvertices,"%d");
		SHOW(model->nfaces,"%d");

		exit(0);
	}
#endif

#if USE_KBD
	DOSKeyBegin();
#endif

#if 0
	{	
		br_scalar s;
		br_angle a;

		for(s=BR_SCALAR(-1.0) ; s < BR_SCALAR(1.0) ; s+= BR_SCALAR(0.1))  {
			a = BR_ASIN(s);
			printf("%f = %04x (%f)\n",
				BrScalarToFloat(s),
				a,
				BrScalarToFloat(BrAngleToScalar(a)));
		}

		getchar();
	}
#endif

#if 0
	{
		br_pixelmap *pm1, *pm2, *pmpal;
		pmpal = BrPixelmapLoad("stdpal.pix");

		pm1 = BrPixelmapAllocate(BR_PMT_RGB_555, 100, 100, NULL, BR_PMAF_INVERTED);
		pm1->map = pmpal;

		pm2 = BrPixelmapAllocateSub(pm1, 20, 20, 32, 32);
//		pm2->map = pmpal;
		pm2->map = NULL;

		BrPixelmapFill(pm1,184);
		BrPixelmapFill(pm2,255);

		BrPixelmapText(pm1, 2, 2, 48, NULL, "Hello World");
		BrPixelmapText(pm2, 2, 2, 0, NULL, "Goodbye!");

		BrPixelmapRectangle(pm1, 0, 0, pm1->width, pm1->height, 48);
		BrPixelmapRectangle(pm2, 0, 0, pm2->width, pm2->height, 48);

		BrPixelmapSave("pm1.pix", pm1);
		BrPixelmapSave("pm2.pix", pm2);

		exit(10);
	}
#endif

	screen_buffer = DOSGfxBegin(NULL);
	BrZbBegin(screen_buffer->type, BR_PMT_DEPTH_16);

	max_debug_lines = screen_buffer->height/(BrPixelmapTextHeight(screen_buffer,NULL)+1);

	/*
	 * Setup CLUT (ignored in true-colour)
	 */
	pm = BrPixelmapLoad("stdpal.pix");
	if(pm)
		DOSGfxPaletteSet(pm);

#if 1
	/*
	 * Load materials from material script
	 */
	BrTableFindHook(LoadTableFFHook);
	BrMapFindHook(LoadMapFFHook);

	n = BrFmtScriptMaterialLoadMany("test.mat",mats,BR_ASIZE(mats));
	BrMaterialAddMany(mats,n);

	/*
	 * Lookup some useful materials for pick testing
	 */
	pick_material = BrMaterialFind("red_flat");
	axis_material = BrMaterialFind("yellow_flat");
	axis_material_os = BrMaterialFind("cyan_flat");
	animate_material = BrMaterialFind("rock");
#endif
	/*
	 * Replace default models with any command line names
	 */
	if(argc > 1)
		TestModels[0].name = argv[1];

	if(argc > 2)
		TestModels[0].material = argv[2];

	/*
	 * Load up models
	 */
	for(i=0; i< BR_ASIZE(TestModels); i++) {
		TestModels[i].model = BrModelLoad(TestModels[i].name);

		if(TestModels[i].model == NULL)
			BR_ERROR1("Could not load '%s'",TestModels[i].name);
		
		if(TestModels[i].material) {
			if(!strcmp(TestModels[i].material,"DEFAULT"))
				mat=NULL;
			else
				mat=BrMaterialFind(TestModels[i].material);

			for(j=0; j< TestModels[i].model->nfaces; j++)
				TestModels[i].model->faces[j].material = mat;
		}


#if 0
		TestModels[i].model->flags |= BR_MODF_CUSTOM;
		TestModels[i].model->custom = LabelModelCallback;
		TestModels[i].model->user = TestModels[i].name;
#endif

#if 0
		TestModels[i].model->flags |= BR_MODF_QUICK_UPDATE;
#endif

#if 0
		TestModels[i].model->flags |= BR_MODF_KEEP_ORIGINAL;
#endif

		BrModelAdd(TestModels[i].model);
	}
#if 0
	_track_dump("mem1.txt","After Model Setup");
#endif

	/*
	 * Some modelsuseful models for interaction
	 */
	light_model = BrModelLoad("arrow.dat");
	axis_model = BrModelLoad("axis1.dat");
	cube_model = BrModelLoad("bbox.dat");

	BrModelAdd(light_model);
	BrModelAdd(axis_model);
	BrModelAdd(cube_model);

	/*
	 * Create the world
	 */
	test_world = BrActorAllocate(BR_ACTOR_NONE,NULL);
	test_world->identifier = "Root";


#if 0
	test_shape_l1 = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));

	test_shape_l1->t.type = BR_TRANSFORM_LOOK_UP;

	BrVector3SetFloat(&test_shape_l1->t.t.look_up.look,2.0,0,0);
	BrVector3SetFloat(&test_shape_l1->t.t.look_up.up,0.0,1.0,0.0);
	BrVector3SetFloat(&test_shape_l1->t.t.look_up.t,-3.0,0,0);

	test_shape_l1->model = axis_model;
	test_shape_l1->material = BrMaterialFind("red_flat");

	test_shape_l2 = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));

	test_shape_l2->t.type = BR_TRANSFORM_LOOK_UP;

	BrVector3SetFloat(&test_shape_l2->t.t.look_up.look,2.0,0,0);
	BrVector3SetFloat(&test_shape_l2->t.t.look_up.up,0.0,1.0,0.0);
	BrVector3SetFloat(&test_shape_l2->t.t.look_up.t,+3.0,0,0);

	test_shape_l2->model = axis_model;
	test_shape_l2->material = BrMaterialFind("red_flat");
#endif

#if 0
	a = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));

	BrMatrix34Translate(&a->t.t.mat,BR_SCALAR(0),BR_SCALAR(-2.5),BR_SCALAR(0.0));
	BrMatrix34PreScale(&a->t.t.mat,BR_SCALAR(5.0),BR_SCALAR(0.5),BR_SCALAR(5.0));
	
	a->model = NULL ;
	a->material = BrMaterialFind("blue_flat");
	a->identifier = "Table";
#endif


	test_shape = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	test_shape->model = TestModels[CurrentModel].model;
	test_shape->identifier = "Test-Shape";

#if 0
	{
		br_model *models[20];
		int n,o;
		
		o = BrWriteModeSet(BR_FS_MODE_TEXT);

		n = BrModelLoadMany("robot.dat",models,20);
		n = BrModelSaveMany("robot.dtx",models,n);
		BrModelAddMany(models,n);

		robot = BrActorLoad("robot.act");
		BrActorSave("robot.atx", robot);

		BrActorAdd(test_shape,robot);

		BrWriteModeSet(o);

		robot_bit = BrActorSearch(robot,"B1.LWO/S1.LWO/F1.LWO/W1.LWO/H1.LWO");
	}
#endif

#if 0
	AddFourChildren(test_shape,3);
#endif

#if 0
	GridActors(test_shape,5,5,5,
		BR_SCALAR(-3.0), BR_SCALAR(-3.0), BR_SCALAR(-3.0),
		BR_SCALAR(1.5), BR_SCALAR(1.5), BR_SCALAR(1.5),BR_ACTOR_MODEL);

	BrMatrix34Scale(&test_shape->t.t.mat,BR_SCALAR(0.16),BR_SCALAR(0.16),BR_SCALAR(0.16));
#endif

#if 0
	bounds_actor = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));
//	bounds_actor->render_style = BR_RSTYLE_BOUNDING_EDGES;
	bounds_actor->model = cube_model;
	bounds_actor->material = BrMaterialFind("red_flat");
#endif

#if 0
	a = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	a->model = axis_model;
#endif

#if 0
	nmorph_models = BrModelLoadMany(TestModels[0].name,morph_models,BR_ASIZE(morph_models));
#endif

	/*
	 * Observer
	 */
	observer = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_CAMERA,&camera_data));
	observer->identifier = "Observer";

	/*
	 * A light
	 */

	light = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_LIGHT,&light_data));
	light->identifier = "Light";

	/*
	 * Add an arrow to the light (pointing down -ve Z)
	 */
	light_child = BrActorAdd(light,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	BrMatrix34Scale(&light_child->t.t.mat,BR_SCALAR(0.2),BR_SCALAR(0.2),BR_SCALAR(-0.2));
	light_child->model = light_model;
	light_child->type = BR_ACTOR_NONE;
	light_child->identifier = "Light-Shape";

	BrLightEnable(light);

	BrEnvironmentSet(test_world);

	/*
	 * Some useful transforms for later
	 */
	BrMatrix34Identity(&Rotation);
	BrMatrix34Identity(&ObsRotation);
	BrMatrix34Identity(&LightRotation);


	DOSMouseBegin();

#if USE_DIVTRAP
	DOSDivTrapBegin();
#endif

	if(Timing)
		InteractiveTimingLoop();
	else
		InteractiveLoop();

#if USE_DIVTRAP
	DOSDivTrapEnd();
#endif

#if USE_KBD
	DOSKeyEnd();
#endif
	DOSGfxEnd();

#if 0
	BrResDumpAll("res.txt");
#endif

	BrZbEnd();
	BrEnd();


	return 0;
}

void dprintf(int x, int y, char *fmt,...)
{
	char temp[256];
 	va_list args;
	int o = 0;
	/*
	 * Build output string
	 */
	va_start(args,fmt);
	vsprintf(temp,fmt,args);
	va_end(args);
	
	BrPixelmapText(back_buffer, x * 4, y * 6, 255, NULL,  temp);
}

/*
 * Find Failed callbacks to automatically load textures & tables
 */
br_pixelmap * BR_CALLBACK LoadMapFFHook(char *name)
{
	br_pixelmap *pm;

	if((pm = BrPixelmapLoad(name)) != NULL) {
		pm->identifier = BrMemStrDup(name);
		BrMapAdd(pm);
	}

	return pm;
}

br_pixelmap * BR_CALLBACK LoadTableFFHook(char *name)
{
	br_pixelmap *pm;

	if((pm = BrPixelmapLoad(name)) != NULL) {
		pm->identifier = BrMemStrDup(name);
		BrTableAdd(pm);
	}

	return pm;
}

br_model * BR_CALLBACK LoadModelFFHook(char *name)
{
	br_model *m;

	if((m = BrModelLoad(name)) != NULL) {
		m->identifier = BrMemStrDup(name);
		BrModelAdd(m);
	}

	return m;
}

void InteractiveLoop(void)
{
	long start_time=0,end_time=100000;
	int total_faces = 0;
	int time_count = 0;
	br_angle a = 0;
	br_scalar s;

	/*
 	 * Allocate off-screen buffer and Z buffer
	 */
	back_buffer = BrPixelmapMatch(screen_buffer, BR_PMMATCH_OFFSCREEN);
	depth_buffer = BrPixelmapMatch(back_buffer, BR_PMMATCH_DEPTH_16);


	/*
	 * Render frames while not quit
	 */

	for(;;) {
		BrPixelmapFill(back_buffer,0);
		BrPixelmapFill(depth_buffer,0xFFFFFFFF);

		test_shape->model = TestModels[CurrentModel].model;

		/*
		 * Modify scene according to controls
		 */
		BrMatrix34Translate(&light->t.t.mat,light_x,light_y,light_z);
		BrMatrix34Post(&light->t.t.mat,&LightRotation);

		BrMatrix34Translate(&observer->t.t.mat,obs_x,obs_y,obs_z);
		BrMatrix34Post(&observer->t.t.mat,&ObsRotation);

		BrMatrix34Translate(&test_shape->t.t.mat,pos_x,pos_y,pos_z);
		BrMatrix34Pre(&test_shape->t.t.mat,&Rotation);
	
		s = BrFixedToScalar(1 << ModelScale);
		BrMatrix34PreScale(&test_shape->t.t.mat,BR_SCALAR(1.0),s,BR_SCALAR(1.0));

#if 0
		/*
		 * Test finding bounds of a hierachy
		 */
		bounds_actor->type = BR_ACTOR_NONE;
		BrActorToBounds(&test_bounds, test_world);
		BrBoundsToMatrix34(&bounds_actor->t.t.mat, &test_bounds);
		bounds_actor->type = BR_ACTOR_MODEL;
#endif

#if 0
		/*
		 * Test robot pivots...
		 */
		if(robot_bit) {
			BrMatrix34PreRotateY(&robot_bit->t.t.mat,BR_ANGLE_DEG(1.0));
		}
#endif

#if 1
		/*
		 * Test U,V animation
		 */
		counter+=BR_ANGLE_DEG(3);
	
		if(animate_material) {
			animate_material->map_transform.m[0][0] = BR_ADD(BR_CONST_DIV(BR_SIN(counter),4),BR_SCALAR(0.75));
			animate_material->map_transform.m[1][1] = BR_ADD(BR_CONST_DIV(BR_COS(counter),4),BR_SCALAR(0.75));
		};
#endif

#if 0
	/*
	 * Test Look-Up
	 */
	BrVector3Sub(&test_shape_l1->t.t.look_up.look,&test_shape->t.t.translate.t,&test_shape_l1->t.t.translate.t);
	BrVector3Sub(&test_shape_l2->t.t.look_up.look,&test_shape->t.t.translate.t,&test_shape_l2->t.t.translate.t);
#endif

#if 0
		/*
		 * Test multiple morph
		 */
		if(ToggleFlag) {
			br_scalar alpha;
			br_float time;

			time = CLOCKREAD() / (float)CLOCK_RATE;
			time = time / 2;
			time = fmod(time , 2);

			if(time > 1.0)
				time = 2.0 - time;

//			alpha = BR_CONST_DIV(BR_SIN(BrScalarToAngle(BrFloatToScalar(time)))+BR_SCALAR(1.0),2);

			alpha = BrFloatToScalar(time);

			MorphModelsMany(TestModels[0].model,morph_models,nmorph_models,alpha);

			BrModelUpdate(TestModels[0].model,BR_MODU_NORMALS | BR_MODU_BOUNDING_BOX);
		}
#endif


		BrZbSceneRender(test_world,observer,back_buffer,depth_buffer);
	
		if(DebugFlag) {
#if 0
			DebugInfoMatrices();
#endif
#if 0
			DebugInfoDebugMatrices();
#endif
#if 0
			DebugInfoVertices();
#endif
#if 0
			DebugInfoGroups();
#endif
#if 0
			DebugInfoTriangle();
#endif
#if 0
			DebugInfoTriangleCount();
#endif
#if 0
			DebugInfoRotations();
#endif
#if 0
			DebugInfoMisc();
#endif
#if 0
			DebugInfoClipPlanes();
#endif
#if 0
			DebugInfoRenderIdents();
#endif
#if 0
			DebugInfoBounds();
#endif
#if 0
			DebugInfoZConvert();
#endif
#if 0
			DebugInfoKeyMatrix();
#endif
	}

#if 1
		PickDisplay();
#endif

		BrPixelmapDoubleBuffer(screen_buffer,back_buffer);

		
		if(TstControls())
			break;
	}
}

void InteractiveTimingLoop(void)
{
	long start_time=0,end_time=100000;
	int total_faces = 0;
	int time_count = 0;
	br_pixelmap *subscreen;


	/*
 	 * Allocate off-screen buffer and Z buffer
	 */
	subscreen = BrPixelmapAllocateSub(screen_buffer,0,10,
		screen_buffer->width,screen_buffer->height-10);

	back_buffer = BrPixelmapMatch(subscreen, BR_PMMATCH_OFFSCREEN);
	depth_buffer = BrPixelmapMatch(back_buffer, BR_PMMATCH_DEPTH_16);


	/*
	 * Modify scene according to controls
	 */
  	BrMatrix34RotateX(&Rotation,BR_ANGLE_DEG(90));
	pos_x = 0; pos_y = 0; pos_z = 0;
	obs_x = 0; obs_y = 0; obs_z = BR_SCALAR(10.0);

	BrMatrix34Translate(&observer->t.t.mat,obs_x,obs_y,obs_z);
	BrMatrix34Post(&observer->t.t.mat,&ObsRotation);

	BrMatrix34Translate(&test_shape->t.t.mat,pos_x,pos_y,pos_z);
	BrMatrix34Pre(&test_shape->t.t.mat,&Rotation);

	test_shape->model = TestModels[CurrentModel].model;

	/*
	 * Render frames while not quit
	 */
	for(;;) {

		BrPixelmapFill(back_buffer,0);
		BrPixelmapFill(depth_buffer,0xFFFFFFFF);

		if(time_count-- == 0) {
			int i;

			end_time = CLOCKREAD();

			BrPixelmapRectangleFill(screen_buffer,0,0,screen_buffer->width,10,24);

			BrPixelmapTextF(screen_buffer,0,0,255,NULL,
				"Frames/Sec = %16g Polys/Sec = %16g",
				TIMING_FRAMES / ((end_time-start_time)/(double)CLOCK_RATE),
				total_faces / ((end_time-start_time)/(double)CLOCK_RATE) );

			time_count = TIMING_FRAMES;
			total_faces = 0;
			start_time = CLOCKREAD();
	  				
		} else
			total_faces += test_shape->model->nfaces;

#if 0
		/*
		 * Test morph
		 */
		if(ToggleFlag) {
			counter+=BR_ANGLE_DEG(3);

			MorphModels(TestModels[0].model,morph_model_1,morph_model_2,
				BR_MUL(BR_SIN(counter),BR_SCALAR(1.5)));

			BrModelUpdate(TestModels[CurrentModel].model,BR_MODU_NORMALS | BR_MODU_BOUNDING_BOX);
		}

#endif

#if 1
		/*
		 * Modify scene according to controls
		 */
		BrMatrix34Translate(&light->t.t.mat,light_x,light_y,light_z);
		BrMatrix34Post(&light->t.t.mat,&LightRotation);

		BrMatrix34Translate(&observer->t.t.mat,obs_x,obs_y,obs_z);
		BrMatrix34Post(&observer->t.t.mat,&ObsRotation);

		BrMatrix34Translate(&test_shape->t.t.mat,pos_x,pos_y,pos_z);
		BrMatrix34Pre(&test_shape->t.t.mat,&Rotation);

		test_shape->model = TestModels[0].model;
#endif

		BrZbSceneRender(test_world,observer,back_buffer,depth_buffer);
		BrPixelmapDoubleBuffer(subscreen,back_buffer);

		if(TstControls())
			break;
	}
}

/*
 * All the controls for 3D test harness
 */
br_int_32 mouse_x, mouse_y;
br_uint_32 mouse_buttons;

#define MSCALE BR_SCALAR(0.006)

void ObjectControls(void)
{
	br_matrix34 tmpmat;

	if(mouse_buttons & BR_MSM_BUTTONL) {

		/*
		 * Drag object around
		 */
		pos_x += BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		pos_y -= BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else if(mouse_buttons & BR_MSM_BUTTONR) {

		/*
		 * Drag object around
		 */

		pos_x += BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		pos_z += BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else {
		/*
		 * Rotate object via rolling ball interface
		 */
		BrMatrix34RollingBall(&tmpmat,mouse_x,-mouse_y,1000);
		BrMatrix34Post(&Rotation,&tmpmat);
	}
}


void ObserverControls(void)
{
	br_matrix34 tmpmat;

	if(mouse_buttons & BR_MSM_BUTTONL) {

		/*
		 * Drag observer around
		 */
		obs_x -= BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		obs_y -= BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else if(mouse_buttons & BR_MSM_BUTTONR) {

		/*
		 * Drag observer around
		 */

		obs_x -= BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		obs_z += BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else {
		/*
		 * Rotate observer via rolling ball interface
		 */
		BrMatrix34RollingBall(&tmpmat,-mouse_x,mouse_y,800);
		BrMatrix34Pre(&ObsRotation,&tmpmat);
	}
}

void LightControls(void)
{
	br_matrix34 tmpmat;

	if(mouse_buttons & BR_MSM_BUTTONL) {

		/*
		 * Drag light around
		 */
		light_x += BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		light_y -= BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else if(mouse_buttons & BR_MSM_BUTTONR) {

		/*
		 * Drag light around
		 */

		light_x += BR_MUL(BR_SCALAR(mouse_x),MSCALE);
		light_z += BR_MUL(BR_SCALAR(mouse_y),MSCALE);

	} else {
		/*
		 * Rotate light via rolling ball interface
		 */
		BrMatrix34RollingBall(&tmpmat,mouse_x,-mouse_y,800);
		BrMatrix34Pre(&LightRotation,&tmpmat);
	}
}

void CameraControls(void)
{
	if(mouse_buttons & BR_MSM_BUTTONL) {

		camera_data.hither_z -= BR_MUL(BR_SCALAR(mouse_y),BR_SCALAR(0.001));

	} else if(mouse_buttons & BR_MSM_BUTTONR) {

		camera_data.yon_z -= BR_MUL(BR_SCALAR(mouse_y),BR_SCALAR(0.001));

	} else {
	}
}

int BR_CALLBACK BrPickNearestModelCallback(
		br_model *model,
		br_material *material,
		br_vector3 *ray_pos, br_vector3 *ray_dir,
		br_scalar t,
		int f,
		int e,
		int v,
		br_vector3 *p,
		br_vector2 *map,
		br_pick_nearest *pn)
{
#if 1
	/*
	 * See if any texture is transparent at this point
	 */
	if(material && material->colour_map && !(material->flags & BR_MATF_DECAL)) {
		br_vector2 t;
		int u,v;
		char *cp;
		br_pixelmap *cm = material->colour_map;

		BrMatrix23ApplyP(&t,map,&material->map_transform);
		t.v[0] = t.v[0] & 0xffff;
		t.v[1] = t.v[1] & 0xffff;
		u = BrScalarToInt(BR_MUL(t.v[0],BrIntToScalar(cm->width)));
		v = BrScalarToInt(BR_MUL(t.v[1],BrIntToScalar(cm->height)));

		cp = (char *)cm->pixels +(cm->base_y+v) * cm->row_bytes + cm->base_x + u;

		if(*cp == 0)
			return 0;
	}
#endif


	if(t < pn->t) {
		pn->t = t;
		pn->actor = pn->temp_actor;
		pn->model = model;
		pn->material = material;
		pn->point = *p;
		pn->face = f;
		pn->edge = e;
		pn->vertex = v;
		pn->map = *map;
	}

	return 0;
}

/*
 * Test callback
 */
int BR_CALLBACK BrPickNearestCallback(
		br_actor *actor,
		br_model *model,
		br_material *material,
		br_vector3 *ray_pos, br_vector3 *ray_dir,
		br_scalar t_near, br_scalar t_far,
		br_pick_nearest *pn)
{
	PickCount++;
	pn->temp_actor = actor;

	BrModelPick2D(model, material, ray_pos, ray_dir, t_near, t_far, BrPickNearestModelCallback, pn);

	return 0;
}

void UserPickControls(void)
{
	PickCursorOn = 1;
	PickCursor_X += mouse_x/2;
	PickCursor_Y += mouse_y/2;


	if(mouse_buttons & BR_MSM_BUTTONL) {

		PickCursorOn = 2;
		PickCount = 0;

		PickNearest.model = NULL;
		PickNearest.t = BR_SCALAR_MAX;

		BrScenePick2D(test_world,observer,
			back_buffer,PickCursor_X,PickCursor_Y,BrPickNearestCallback,&PickNearest);

		if(PickNearest.model) {
#if 1
			PickNearest.model->faces[PickNearest.face].material = pick_material;
			BrModelUpdate(PickNearest.model,BR_MODU_ALL);
#endif
#if 0
			PickNearest.actor->render_style = BR_RSTYLE_EDGES;
#endif
#if 0
			if(PickNearest.material && PickNearest.material->colour_map) {
				br_vector2 t;
				int u,v;
				/*
				 * Scribble in the texture
				 */
				BrMatrix23ApplyP(&t,&PickNearest.map,&PickNearest.material->map_transform);
				t.v[0] = t.v[0] & 0xffff;
				t.v[1] = t.v[1] & 0xffff;
				u = BrScalarToInt(BR_MUL(t.v[0],BrIntToScalar(PickNearest.material->colour_map->width)));
				v = BrScalarToInt(BR_MUL(t.v[1],BrIntToScalar(PickNearest.material->colour_map->height)));
				BrPixelmapPlot(PickNearest.material->colour_map,u,v,184);
			}
#endif

		}

	} else if(mouse_buttons & BR_MSM_BUTTONR) {

		PickCursorOn = 3;
		PickCount = 0;

		PickNearest.model = NULL;
		PickNearest.t = BR_SCALAR_MAX;

		BrScenePick2D(test_world,observer,
			back_buffer,PickCursor_X,PickCursor_Y,BrPickNearestCallback,&PickNearest);

		if(PickNearest.model) {
#if 1
			PickNearest.model->faces[PickNearest.face].material = NULL;
			BrModelUpdate(PickNearest.model,BR_MODU_ALL);
#endif
#if 0
			PickNearest.actor->render_style = BR_RSTYLE_DEFAULT;
#endif
		}
	} else {

	}
}

int PickFlag = 0;

void ForceMaterial(br_model *model, br_material *mat)
{
	int i;

	for(i=0; i< model->nfaces; i++) {
		model->faces[i].material = mat;
	}

	BrModelPrepare(model,BR_MPREP_ALL);
}
  
int TstControls(void)
{
	br_matrix34 tmpmat;

	/*
	 * Mouse control
	 */
	DOSMouseRead(&mouse_x,&mouse_y,&mouse_buttons);

#if 0
	if(mouse_buttons & BR_MSM_BUTTONR)
		return 1;
#endif

	PickCursorOn = 0;

#if 0
	PickCursorOn = 3;

	PickCount = BrScenePick2D(test_world,observer,
		&test_colour_buffer,PickCursor_X,PickCursor_Y,PickCallback,NULL);
#endif

#if USE_KBD

#if 1
	if(DOSKeyTest(SC_P,0,0)) {
		UserPickControls();
		mouse_x=0;
		mouse_y=0;
	}
#endif

#if 1
	if(mouse_x !=0 || mouse_y != 0) {

		if(DOSKeyTest(SC_C,0,0))
			CameraControls();
		else if(DOSKeyTest(SC_O,0,0))
			ObserverControls();
		else if(DOSKeyTest(SC_L,0,0))
			LightControls();
		else
			ObjectControls();
		

		mouse_x=0;
		mouse_y=0;
	}
#endif

	/*
	 * Keys
	 */
	if(DOSKeyTest(SC_Q,0,REPT_FIRST_DOWN))
		return 1;

	if(DOSKeyTest(SC_F,0,REPT_FIRST_DOWN))
		ToggleFlag = !ToggleFlag;

	if(DOSKeyTest(SC_I,0,REPT_FIRST_DOWN))
		DebugFlag = !DebugFlag;

	if(DOSKeyTest(SC_A,0,REPT_FIRST_DOWN))
		if(test_shape->render_style < BR_RSTYLE_BOUNDING_FACES)
			test_shape->render_style++;

	if(DOSKeyTest(SC_S,0,REPT_FIRST_DOWN))
		if(test_shape->render_style > 0)
			test_shape->render_style--;

#if 0
	if(DOSKeyTest(SC_A,QUAL_NONE,0)) {
		br_euler e = {0,0,0,BR_EULER_YXZ_R};

		e.a = angle_y;
		e.b = angle_x;
		e.c = angle_z;

		BrEulerToQuat(&slerp_a,&e);
	}

	if(DOSKeyTest(SC_B,QUAL_NONE,0)) {
		br_euler e = {0,0,0,BR_EULER_YXZ_R};

		e.a = angle_y;
		e.b = angle_x;
		e.c = angle_z;

		BrEulerToQuat(&slerp_b,&e);
	}
#endif

#if 0
	if(DOSKeyTest(SC_N,QUAL_NONE,REPT_FIRST_DOWN))
		quat_spins++;

	if(DOSKeyTest(SC_M,QUAL_NONE,REPT_FIRST_DOWN))
		quat_spins--;
#endif

	if(DOSKeyTest(SC_N,QUAL_NONE,REPT_FIRST_DOWN))
		if(ModelScale > 1)
			ModelScale--;

	if(DOSKeyTest(SC_M,QUAL_NONE,REPT_FIRST_DOWN))
		if(ModelScale < 32)
			ModelScale++;

	if(DOSKeyTest(SC_COMMA,QUAL_NONE,REPT_FIRST_DOWN))
		if(CurrentModel > 0)
			CurrentModel--;

	if(DOSKeyTest(SC_DOT,QUAL_NONE,REPT_FIRST_DOWN))
		if(CurrentModel < (BR_ASIZE(TestModels)-1))
			CurrentModel++;

	if(DOSKeyTest(SC_D,0,REPT_FIRST_DOWN))
		test_shape = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));

	if(DOSKeyTest(SC_P,0,REPT_FIRST_DOWN))
		PrelightTest();

	if(DOSKeyTest(SC_F1,QUAL_NONE,0)) {
		light_data.type = (light_data.type & BR_LIGHT_VIEW) | BR_LIGHT_DIRECT;
		light_child->type = BR_ACTOR_NONE;
	}

	if(DOSKeyTest(SC_F2,QUAL_NONE,0)) {
		light_data.type = (light_data.type & BR_LIGHT_VIEW) | BR_LIGHT_POINT;
		light_child->type = BR_ACTOR_MODEL;
	}

	if(DOSKeyTest(SC_F3,QUAL_NONE,0)) {
		light_data.type = (light_data.type & BR_LIGHT_VIEW) | BR_LIGHT_SPOT;
		light_child->type = BR_ACTOR_MODEL;
	}

	if(DOSKeyTest(SC_F4,0,REPT_FIRST_DOWN)) {
		light_data.type ^= BR_LIGHT_VIEW;
	}

	if(DOSKeyTest(SC_F5,QUAL_NONE,0)) {
		light_data.cone_inner += BR_ANGLE_DEG(1.0);
		light_data.cone_outer = light_data.cone_inner + BR_ANGLE_DEG(5.0);
	}

	if(DOSKeyTest(SC_F6,QUAL_NONE,0)) {
		light_data.cone_inner -= BR_ANGLE_DEG(1.0);
		light_data.cone_outer = light_data.cone_inner + BR_ANGLE_DEG(5.0);
	}

	if(DOSKeyTest(SC_1,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("grey_flat");
	if(DOSKeyTest(SC_1,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("grey");

	if(DOSKeyTest(SC_2,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("red_flat");
	if(DOSKeyTest(SC_2,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("red");

	if(DOSKeyTest(SC_3,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("blue_flat");
	if(DOSKeyTest(SC_3,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("blue");

	if(DOSKeyTest(SC_4,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("green_flat");
	if(DOSKeyTest(SC_4,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("green");

	if(DOSKeyTest(SC_5,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("test_texture");
	if(DOSKeyTest(SC_5,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("solid_red");

	if(DOSKeyTest(SC_6,QUAL_NONE,REPT_FIRST_DOWN))
		test_shape->material = BrMaterialFind("mandrill");
	if(DOSKeyTest(SC_7,QUAL_NONE,REPT_FIRST_DOWN))
		test_shape->material = BrMaterialFind("earth");
	if(DOSKeyTest(SC_8,QUAL_NONE,REPT_FIRST_DOWN))
		test_shape->material = BrMaterialFind("rosewood");
	if(DOSKeyTest(SC_9,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("rock");
	if(DOSKeyTest(SC_9,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("terrain");
	if(DOSKeyTest(SC_0,QUAL_NONE,0))
		test_shape->material = BrMaterialFind("test_environment");
	if(DOSKeyTest(SC_0,QUAL_SHIFT,0))
		test_shape->material = BrMaterialFind("test_environment_1");

#if 0
	if(DOSKeyTest(SC_Z,0,REPT_FIRST_DOWN))
		Pick3DTest();
#endif

	if(DOSKeyTest(SC_Z,0,REPT_FIRST_DOWN))
		BrModelUpdate(TestModels[CurrentModel].model,BR_MODU_ALL);
#else

	if(PickFlag) {
		UserPickControls();
		mouse_x=0;
		mouse_y=0;
	} else 	if(mouse_x !=0 || mouse_y != 0) {

		ObjectControls();

		mouse_x=0;
		mouse_y=0;
	}

	if(KBHIT()) switch(GETKEY() & 0xff) {

	case ',':
		if(CurrentModel > 0)
			CurrentModel--;
		break;

	case '.':
		if(CurrentModel < (BR_ASIZE(TestModels)-1))
			CurrentModel++;
		break;

	case 'a':
		if(test_shape->render_style < BR_RSTYLE_BOUNDING_FACES)
			test_shape->render_style++;
		break;

	case 's':
		if(test_shape->render_style > 0)
			test_shape->render_style--;
		break;
	
	case 'd':
		test_shape = BrActorAdd(test_world,BrActorAllocate(BR_ACTOR_MODEL,NULL));
		break;

	case '1':
		test_shape->material = BrMaterialFind("grey_flat");
		break;
	case '2':
		test_shape->material = BrMaterialFind("red_flat");
		break;
	case '3':
		test_shape->material = BrMaterialFind("blue_flat");
		break;
	case '4':
		test_shape->material = BrMaterialFind("green_flat");
		break;
	case '!':
		test_shape->material = BrMaterialFind("grey");
		break;
	case '"':
		test_shape->material = BrMaterialFind("red");
		break;
	case 'œ':
		test_shape->material = BrMaterialFind("blue");
		break;
	case '$':
		test_shape->material = BrMaterialFind("green");
		break;
	case '5':
		test_shape->material = BrMaterialFind("test_texture");
		break;
	case '6':
		test_shape->material = BrMaterialFind("mandrill");
		break;
	case '7':
		test_shape->material = BrMaterialFind("earth");
		break;
	case '8':
		test_shape->material = BrMaterialFind("rosewood");
		break;
	case '9':
		test_shape->material = BrMaterialFind("rock");
		break;
	case '0':
		test_shape->material = BrMaterialFind("test_environment");
		break;

	case 'q':
		return 1;

	case 'v':
		light_data.type ^= BR_LIGHT_VIEW;
		break;

	case 'f':
		ToggleFlag = !ToggleFlag;
		break;
#if 0
	case 'p':
		PickFlag = !PickFlag;
		break;
#endif

	case 'p':
		PrelightTest();

	case 'i':
		DebugFlag = !DebugFlag;
		break;

	case 'z':
		Pick3DTest();
		break;
	}
#endif
	return 0;
}

void BR_CALLBACK GetM2SCallback(
	br_actor *actor,
	br_model *model,
	br_material *material,
	br_uint_8	style,
	br_matrix4 *model_to_screen,
	br_int_32 *bounds)
{
	DebugMatrix3 = *model_to_screen;
}

void PrintMatrix34(int x,int y,br_matrix34 *t)
{
	int i,j;

	for(i=0; i<3; i++)
		for(j=0; j<4; j++)
			dprintf(x+i*12,y+j*2,"%11.5f",BrScalarToFloat(t->m[j][i]));
}

void PrintMatrix4(int x,int y,br_matrix4 *t)
{
	int i,j;

	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
#if 1
			dprintf(x+i*12,y+j*2,"%11.5f",BrScalarToFloat(t->m[j][i]));
#else
			dprintf(x+i*12,y+j*2,"%08x",BrScalarToFixed(t->m[j][i]));
#endif

}

void DebugInfoMatrices(void)
{
 	PrintMatrix4(0, 15,&fw.model_to_screen);
 	PrintMatrix34(0,24,&fw.model_to_view);
}

void DebugInfoDebugMatrices(void)
{
 	PrintMatrix4(0,  4,&DebugMatrix3);
 	PrintMatrix4(0, 14,&DebugMatrix4);
 	PrintMatrix4(0, 24,&DebugMatrix5);
}

void DebugInfoGroups(void)
{
	int i,j;
	br_face_group *fgp = zb.model->face_groups;
	br_vertex_group *vgp = zb.model->vertex_groups;

	for(i=0,j=1; i< zb.model->nface_groups; i++,fgp++,j++) {
		if(j > max_debug_lines)
			continue;

		dprintf(0,j,"F %3d %15s - f=%d count=%d clipped=%d",i,
			fgp->material?fgp->material->identifier:"<DEFAULT>",
			fgp->nfaces,
			zb.face_group_counts[i],
			zb.face_group_clipped[i]);
	}

	j++;

	for(i=0 ; i< zb.model->nvertex_groups; i++,vgp++,j++) {
		if(j > max_debug_lines)
			continue;

		dprintf(0,j,"V %3d %15s - v=%d",i,
			vgp->material?vgp->material->identifier:"<DEFAULT>",
			vgp->nvertices);
	}

}

void DebugInfoVertices(void)
{
	int i,j;
	struct temp_vertex *tvp;
	char tmp[16];

#if 0
	dprintf(0,0,"nvertices = %d\n",zb.model->nprepared_vertices);
#endif

	tvp = zb.temp_vertices;

	for(i=0; i< zb.model->nprepared_vertices; i++,tvp++) {

		if(zb.vertex_counts[i] <= 0)
			continue;

#if 1
		sprintf(tmp,"%d",i);
		BrPixelmapText(back_buffer,ScreenToInt(tvp->v[X]),ScreenToInt(tvp->v[Y])+screen_buffer->base_y,63,NULL,tmp);
#endif
		BrPixelmapPlot(back_buffer,ScreenToInt(tvp->v[X]),ScreenToInt(tvp->v[Y])+screen_buffer->base_y,63);

#if 0 /* Text line per vertex */
		if(i > (max_debug_lines-1))
			continue;

#if 1
		dprintf(0,i+1,"%2d %d:",i,zb.vertex_counts[i]);
#endif

#if 1
		dprintf(7,i+1,"(%6.3f, %6.3f, %6.3f)",
			BrScalarToFloat(ScreenToScalar(tvp->v[X])),
			BrScalarToFloat(ScreenToScalar(tvp->v[Y])),
			BrScalarToFloat(ScreenToScalar(tvp->v[Z])));
#endif

#if 1
		dprintf(40,i+1,"UV=(%6.3f, %6.3f) W=%6.3f",
			BrScalarToFloat(ScreenToScalar(tvp->comp[C_U])),
			BrScalarToFloat(ScreenToScalar(tvp->comp[C_V])),
			BrScalarToFloat(ScreenToScalar(tvp->comp[C_W])));
#endif

#if 0
		dprintf(40,i+1,"%08x",tvp->outcode);
#endif

#if 0
		dprintf(7,i+1,"(%3.3f, %3.3f, %3.3f)",
			BrScalarToFloat(zb.model->prepared_vertices[i].p.v[X]),
			BrScalarToFloat(zb.model->prepared_vertices[i].p.v[Y]),
			BrScalarToFloat(zb.model->prepared_vertices[i].p.v[Z]));
#endif

#if 0
		for(j=0; j < NUM_COMPONENTS ; j++)
			dprintf(3+j*10,i+1,"%11.5f",BrScalarToFloat(tvp->comp[j]));
#endif

#endif

	}
}

void DebugInfoParam(int x, int y, char *name, struct scan_parameter *p)
{
	dprintf(x,y+0,"%s current   : %08x %11.5hf",name,p->current,p->current);
	dprintf(x,y+1, "  d_carry   : %08x %11.5hf",p->d_carry,p->d_carry);
	dprintf(x,y+2, "  d_nocarry : %08x %11.5hf",p->d_nocarry,p->d_nocarry);
	dprintf(x,y+3, "  grad_x    : %08x %11.5hf",p->grad_x,p->grad_x);
	dprintf(x,y+4, "  grad_y    : %08x %11.5hf",p->grad_y,p->grad_y);
}

void DebugInfoTriangle(void)
{
#if 0
	extern int g_divisor; 
	dprintf(0,0,"g_divisor      : %08x %hf", g_divisor, g_divisor);
#endif

	dprintf(0,1,"DOSDivTrapCount() : %d", DOSDivTrapCount(1));
	dprintf(0,2,"ToggleFlag     : %08x", ToggleFlag);

	DebugInfoParam(0, 6,"Z",&zb.pz);
	DebugInfoParam(0,12,"I",&zb.pi);
	DebugInfoParam(0,18,"U",&zb.pu);
	DebugInfoParam(0,24,"V",&zb.pv);
}

void DebugInfoMisc(void)
{
	dprintf(0,27,"Scratch Size   = %d",fw.scratch_size);
	dprintf(0,28,"Scratch Last   = %d",fw.scratch_last);
	dprintf(0,29,"DebugCount0    = %d",DebugCount0);
	dprintf(0,30,"DebugCount1    = %d",DebugCount1);
	dprintf(0,31,"ToggleFlag     = %d",ToggleFlag);
	dprintf(0,32,"MOUSE		     = %02X,%3d,%3d",mouse_buttons,mouse_x,mouse_y);

	DebugCount0 = 0;
	DebugCount1 = 0;
}

void DebugInfoClipPlanes(void)
{
	int c;

	for(c=0; c < fw.nactive_clip_planes; c++) {
		dprintf(0,c,"Clip %d: (%11.5f,%11.5f,%11.5f,%11.5f)",c,
			BrScalarToFloat(fw.active_clip_planes[c].screen_plane.v[0]),
			BrScalarToFloat(fw.active_clip_planes[c].screen_plane.v[1]),
			BrScalarToFloat(fw.active_clip_planes[c].screen_plane.v[2]),
			BrScalarToFloat(fw.active_clip_planes[c].screen_plane.v[3]));
	}
}

void DebugInfoRenderIdents(void)
{
	struct zb_material_type *zbmt;

	dprintf(0,0,"Renderer: [%s]",zb.type->identifier?zb.type->identifier:"NULL");

	if(zb.material) {
		zbmt = zb.material->rptr;

		dprintf(0,1,"Material: [%s]",zb.material->identifier?zb.material->identifier:"NULL");
		dprintf(0,2,"Type    : [%s]",zbmt->identifier?zbmt->identifier:"NULL");
	}
}

void DebugInfoBounds(void)
{
	int i;

	for(i=0; i < 3; i++) {
		dprintf(0,1+i,"Min: %g",BrScalarToFloat(test_bounds.min.v[i]));
		dprintf(15,1+i,"Max: %g",BrScalarToFloat(test_bounds.max.v[i]));
	}
}

void DebugInfoKeyMatrix(void)
{
	int i;
	extern char BR_PUBLIC_ENTRY DOSKeyMap(unsigned char scancode);

	for(i=0; i < 16;i++) {
		BrPixelmapTextF(back_buffer, 16+i*16, 18, 88, NULL, "%02X",i);
		BrPixelmapTextF(back_buffer, 0, 24+6*i,   88, NULL, "%1X0",i);
	}

	for(i=0; i < 256;i++) {
		BrPixelmapTextF(back_buffer, (4+(i % 16) * 4)*4,(4 + i/16)*6, DOSKeyMap(i) & 1?184:255, NULL, "%02X",DOSKeyMap(i));
	}	
}

#if 0
int ConvertZViewToScreen(br_scalar vz, br_scalar hither, br_scalar yon)
{
	br_scalar z;

	/*
	 * Work out homogenous screen Z
	 */
	z = BR_MULDIV(vz,(yon+hither),(yon-hither)) + BR_CONST_MUL(BR_MULDIV(yon,hither,(yon-hither)),2);

	/*
	 * Divide out to get Z buffer value (16 bit)
	 */
	return (int)(BrScalarToFloat(z) * -32768.0 / BrScalarToFloat(-vz));
}
#else
int ConvertZViewToScreen(br_scalar vz, br_scalar hither, br_scalar yon)
{
	br_matrix4 screen_xfm;
	br_vector3 view_point = BR_VECTOR3(0,0,0);
	br_vector4 screen_point;
	br_scalar z;

	/*
	 * Get view to screen transform
	 */
	BrMatrix4Perspective(&screen_xfm,BR_ANGLE_DEG(90),BR_SCALAR(1.0),-hither,-yon);

	/*
	 * Push 0,0,vz,1 through transform
	 */
	view_point.v[2] = vz;
	BrMatrix4ApplyP(&screen_point,&view_point,&screen_xfm);

	/*
	 * Divide out to get Z buffer value (16 bit)
	 */
	return (int)(BrScalarToFloat(screen_point.v[2]) * -32768.0 / BrScalarToFloat(screen_point.v[3]));
}
#endif

void DebugInfoZConvert(void)
{
	/*
	 * Generate point in view space
	 */
	br_matrix4 screen_xfm;
	br_matrix34 xfm;
	br_vector3 model_point = BR_VECTOR3(0,0,0.05);
	br_vector3 view_point;
	br_vector4 screen_point;
	br_scalar z;
	int cz;

#if 1
	BrActorToActorMatrix34(&xfm, test_shape, observer);
	BrActorToScreenMatrix4(&screen_xfm, observer, observer);

	BrMatrix34ApplyP(&view_point, &model_point, &xfm);

	BrMatrix4ApplyP(&screen_point, &view_point, &screen_xfm);

	z = BrFloatToScalar(BrScalarToFloat(screen_point.v[2]) * -32768.0 / BrScalarToFloat(screen_point.v[3]));
	
	dprintf(0,32,"Z               = %08x",BrScalarToFixed(z));

	cz = ConvertZViewToScreen(view_point.v[2],camera_data.hither_z,camera_data.yon_z);

	dprintf(0,28,"Converted Z      = %08x",cz);
#endif
}

void PickDisplay(void)
{
	int colour,x,y,i;

	
	if(PickCursorOn) {

		switch(PickCursorOn) {
		case 1:
			colour = 63;
			break;
		case 2:
			colour = 184;
			break;
		case 3:
			colour = 120;
			break;
		}

		x = PickCursor_X + screen_buffer->width/2;
		y = PickCursor_Y + screen_buffer->height/2;
	
		BrPixelmapLine(back_buffer, x+2,y,x+6,y,colour);
		BrPixelmapLine(back_buffer, x-2,y,x-6,y,colour);
		BrPixelmapLine(back_buffer, x,y+2,x,y+6,colour);
		BrPixelmapLine(back_buffer, x,y-2,x,y-6,colour);
		
		dprintf(0,0,"Pick: Count = %d at (%d,%d)",PickCount,PickCursor_X,PickCursor_Y);
		if(PickNearest.model == NULL)
			dprintf(0,1," Face = NONE");
		else
			dprintf(0,1," Face:%d  Edge:%d Vertex:%d  T=%f Map=(%f,%f)",
				PickNearest.face,PickNearest.edge,PickNearest.vertex,
				BrScalarToFloat(PickNearest.t),
				BrScalarToFloat(PickNearest.map.v[0]),
				BrScalarToFloat(PickNearest.map.v[1]));
	}
}

#define C4_TRANSLATE 0.82
#define C4_SCALE 0.5
void AddFourChildren(br_actor *p, int level)
{
	br_actor *a;

	level--;

	a = BrActorAdd(p,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	BrMatrix34Translate(&a->t.t.mat,BR_SCALAR( C4_TRANSLATE),BR_SCALAR( C4_TRANSLATE),BR_SCALAR(C4_TRANSLATE));
	BrMatrix34PreScale(&a->t.t.mat,BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE));
	a->identifier = "Child-1";
	if(level > 0)
		AddFourChildren(a,level);

	a = BrActorAdd(p,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	BrMatrix34Translate(&a->t.t.mat,BR_SCALAR( C4_TRANSLATE),BR_SCALAR(-C4_TRANSLATE),BR_SCALAR(C4_TRANSLATE));
	BrMatrix34PreScale(&a->t.t.mat,BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE));
	a->identifier = "Child-2";
	if(level > 0)
		AddFourChildren(a,level);

	a = BrActorAdd(p,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	BrMatrix34Translate(&a->t.t.mat,BR_SCALAR(-C4_TRANSLATE),BR_SCALAR( C4_TRANSLATE),BR_SCALAR(C4_TRANSLATE));
	BrMatrix34PreScale(&a->t.t.mat,BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE));
	a->identifier = "Child-3";
	if(level > 0)
		AddFourChildren(a,level);

	a = BrActorAdd(p,BrActorAllocate(BR_ACTOR_MODEL,NULL));
	BrMatrix34Translate(&a->t.t.mat,BR_SCALAR(-C4_TRANSLATE),BR_SCALAR(-C4_TRANSLATE),BR_SCALAR(C4_TRANSLATE));
	BrMatrix34PreScale(&a->t.t.mat,BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE),BR_SCALAR(C4_SCALE));
	a->identifier = "Child-4";
	if(level > 0)
		AddFourChildren(a,level);
}

void GridActors(br_actor * parent,
	int nx, int ny, int nz,
	br_scalar sx, br_scalar sy, br_scalar sz,
	br_scalar dx, br_scalar dy, br_scalar dz, int t)
{
	int i,j,k;
	br_actor *a;
	br_scalar x,y,z;

	for(z = sz, k=0; k < nz; k++, z = BR_ADD(z,dz)) {
		for(y = sy, j=0; j < ny; j++, y = BR_ADD(y,dy)) {
			for(x = sx, i=0; i < nx; i++, x = BR_ADD(x,dx)) {
				a = BrActorAdd(parent,BrActorAllocate(t,NULL));
				BrMatrix34Translate(&a->t.t.mat,x,y,z);
			}
		}
	}
}

/*
 * Custom model callback
 */
void BR_CALLBACK LabelModelCallback(
				br_actor *actor,
				br_model *model,
				br_material *material,
				br_uint_8 style,
				int on_screen,
				br_matrix34 *model_to_view,
				br_matrix4 *model_to_screen)
{
	br_vector4 point;
	br_vector3 mp[4] = {
		BR_VECTOR3(0.0,0.0,0.05),
		BR_VECTOR3(1,1,1),
		BR_VECTOR3(-1,-1,-1),
		BR_VECTOR3(1,-1,1),
	};
	br_vector2 sp[4];
	br_vector3 spz[4];
	br_uint_32 oc[4];
	char tmp[80];
	int i;

	if(!ToggleFlag) {
		BrZbModelRender(actor, model, material, style, on_screen, 0);
		return;
	}

	/*
	 * Add an axis shape to model
	 */
	BrZbModelRender(actor, axis_model,
					(on_screen == OSC_ACCEPT)?axis_material_os:axis_material,
					BR_RSTYLE_FACES, OSC_PARTIAL, 0);

	/*
	 * Render model, Ignoring any custom flags
	 */	
	BrZbModelRender(actor, model, material, style, on_screen, 0);

#if 1
	{
		extern br_vector3 pick_point;

		oc[0] = BrPointToScreenXYZO(spz,&PickNearest.point);
		if(!(oc[0] & OUTCODES_ALL))
			BrPixelmapPlot(back_buffer, BrScalarToInt(spz[0].v[0]),BrScalarToInt(spz[0].v[1]),184);
	}
#endif

#if 0
	if(BrOriginToScreenXY(sp))
		return;

	BrPixelmapText(back_buffer,BrScalarToInt(sp->v[0]),BrScalarToInt(sp->v[1]),63,NULL,model->user);
#endif

#if 0
	oc[0] = BrPointToScreenXYZO(spz,mp);

#if 1
	sprintf(tmp,"(%08x, %08x, %08x)",
		BrScalarToFixed(spz[0].v[0]),
		BrScalarToFixed(spz[0].v[1]),
		BrScalarToFixed(spz[0].v[2]));
#endif

#if 0
	sprintf(tmp,"(%4.2f,%4.2f,%4.2f) = (%08x, %08x, %08x)",
		BrScalarToFloat(mp[0].v[0]),
		BrScalarToFloat(mp[0].v[1]),
		BrScalarToFloat(mp[0].v[2]),
		BrScalarToFixed(spz[0].v[0]),
		BrScalarToFixed(spz[0].v[1]),
		BrScalarToFixed(spz[0].v[2]));
#endif

	BrPixelmapPlot(back_buffer, BrScalarToInt(spz[0].v[0]),BrScalarToInt(spz[0].v[1]),184);
	BrPixelmapText(back_buffer, BrScalarToInt(spz[0].v[0])+2,BrScalarToInt(spz[0].v[1])-2,63,BrFontProp4x6,tmp);

	BrPixelmapText(back_buffer,0,20+8,63,BrFontProp4x6,tmp);
#endif

}

void MorphModels(br_model *dest, br_model *model_a, br_model *model_b, br_scalar alpha)
{
	br_scalar beta = BR_SUB(BR_SCALAR(1.0),alpha);
	br_vertex *vp_dest,*vp_a,*vp_b;
	int v;
	
	if(dest->vertex_tags == NULL)
		BR_ERROR0("Model has no vertex tags");

	vp_dest = dest->vertices;
	vp_a = model_a->vertices;
	vp_b = model_b->vertices;

	for(v=0; v < dest->nvertices; v++, vp_dest++, vp_a++, vp_b++) {
		vp_dest->p.v[0] = BR_MAC2(vp_a->p.v[0],beta,vp_b->p.v[0],alpha);
		vp_dest->p.v[1] = BR_MAC2(vp_a->p.v[1],beta,vp_b->p.v[1],alpha);
		vp_dest->p.v[2] = BR_MAC2(vp_a->p.v[2],beta,vp_b->p.v[2],alpha);
	}
}

void MorphModelsMany(br_model *dest, br_model **models, int nmodels, br_scalar alpha)
{
	br_scalar a;
	int mnum;
	
	nmodels--;

	if(alpha <= BR_SCALAR(0.0)) {
		a = alpha;
		mnum = 0;
	} else if(alpha >= BR_SCALAR(1.0)) {
		a = alpha;
		mnum = nmodels-1;
	} else {
		mnum = BrScalarToInt(BR_MUL(alpha,BrIntToScalar(nmodels)));
		a = BR_SUB(BR_MUL(alpha,BrIntToScalar(nmodels)),BrIntToScalar(mnum));
	}

	ASSERT(mnum < nmodels);

	MorphModels(dest,models[mnum],models[mnum+1],a);
}

br_uint_32 BR_CALLBACK ClearRenderStyle(br_actor *a, void *dummy)
{
	a->render_style = BR_RSTYLE_DEFAULT;

	BrActorEnum(a,ClearRenderStyle,dummy);
	
	return 0;
}


int BR_CALLBACK Pick3DCallback(
		br_actor *a,
		br_model *model,
		br_material *material,
		br_matrix34 *transform,
		br_bounds *bounds,
		void *arg)
{
	if(a != test_shape)
		a->render_style = BR_RSTYLE_EDGES;

	return 0;
}


void Pick3DTest(void)
{
	/*
	 * Go through whole world and set render style to default
	 */
	ClearRenderStyle(test_world,NULL);

	/*
	 * Do a pick3d that sets the render style to wireframe
	 */
	BrScenePick3D(test_world,test_shape,&(test_shape->model->bounds),Pick3DCallback,NULL);
}

void PrelightTest(void)
{
	BrZbSceneRenderBegin(test_world,observer,back_buffer,depth_buffer);
	BrSceneModelLight(test_shape->model,test_shape->material,test_world,test_shape);
	BrZbSceneRenderEnd();
}


#if 0

#define D(x) ((((x)-1) * 65536L)/16)
/*
 * Magic square - best so far
 */
br_fixed_ls dither[4][4] = {
	{ D( 1), D( 7), D(10), D(16) },
	{ D(12), D(14), D( 3), D( 5) },
	{ D( 8), D( 2), D(15), D( 9) },
	{ D(13), D(11), D( 6), D( 4) },
};
#endif

#if 1

#define D(x) ((((x)) * 65536L)/4)

/*
 * Fake 2x2
 */
br_fixed_ls dither[4][4] = {
	{ D( 0), D( 2), D( 0), D( 2) },
	{ D( 3), D( 1), D( 3), D( 1) },
	{ D( 0), D( 2), D( 0), D( 2) },
	{ D( 3), D( 1), D( 3), D( 1) },
};

#endif

#if 0
/*
 * Ordered dither - so so
 */
br_fixed_ls dither[4][4] = {
	{ D( 1), D( 9), D( 3), D(11) },
	{ D(13), D( 5), D(15), D( 7) },
	{ D( 4), D(12), D( 2), D(10) },
	{ D(16), D( 8), D(14), D( 6) },
};
#endif

#if 0
/*
 * Expanding spiral - Horrible
 */
br_fixed_ls dither[4][4] = {
	{ D( 7), D( 8), D( 9), D(10) },
	{ D( 6), D( 1), D( 2), D(11) },
	{ D( 5), D( 4), D( 3), D(12) },
	{ D(16), D(15), D(14), D(13) },
};
#endif

#if 0

/*
 * Override smooth shaded 16 bit Z buffered triangles
 */
void BR_ASM_CALL TestPixelRender_Z2ID(br_int_32 x, br_int_32 y)
{
	SETUP_OFFSET;

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = BrFixedToInt(CURRENT(pi) + dither[x & 3][y & 3]);
	}
}

void BR_ASM_CALL TestPixelRender_Z2I(br_int_32 x, br_int_32 y)
{
	SETUP_OFFSET;

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = CURRENT_INT(pi);
	}
}

void BR_ASM_CALL TriangleRenderPIZ2I(struct temp_vertex *v0, struct temp_vertex *v1,struct temp_vertex *v2)
{
	zb.component_mask = CM_Z | CM_I;
	zb.correct_mask = 0;

	zb.trapezoid_render = GenericTrapezoidRender;
	zb.pixel_render = ToggleFlag?TestPixelRender_Z2I:TestPixelRender_Z2ID;

	GenericTriangleRender(v0,v1,v2);
}
#endif

#if 0
/*
 * Override smooth shaded 16 bit Z buffered triangles RGB 555
 */
void BR_ASM_CALL TestPixelRender_Z2ID_555(br_int_32 x, br_int_32 y)
{
	int offset = (y * zb.row_width/2 + x);
	br_fixed_ls dr,dg,db;

	dr = dg = db = dither[offset & 3][(offset & 0xC0) >> 6];
//	dr = dg = db = dither[x & 3][y & 3];

//	dr = dither[x & 3][y & 3];
//	dg = dither[(x+2) & 3][y & 3];
//	db = dither[x & 3][(y+2) & 3];

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_TRUE16 =
			 ((BrFixedToInt((CURRENT(pr)>>3) + dr) & 0x1f) << 10) |
			 ((BrFixedToInt((CURRENT(pg)>>3) + dg) & 0x1f) << 5) |
			 ((BrFixedToInt((CURRENT(pb)>>3) + db) & 0x1f));
	}
}

void BR_ASM_CALL TestPixelRender_Z2I_555(br_int_32 x, br_int_32 y)
{
	int offset = (y * zb.row_width/2 + x);

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_TRUE16 = ((CURRENT_INT(pr) & 0xF8) << 7) | ((CURRENT_INT(pg) & 0xF8) << 2) | ((CURRENT_INT(pb) & 0xF8) >> 3);
	}
}

void BR_ASM_CALL TriangleRenderPIZ2I_RGB_555(struct temp_vertex *v0, struct temp_vertex *v1,struct temp_vertex *v2)
{
	zb.component_mask = CM_Z | CM_R | CM_G | CM_B;
	zb.correct_mask = 0;

	zb.trapezoid_render = GenericTrapezoidRender;
	zb.pixel_render = ToggleFlag?TestPixelRender_Z2ID_555:TestPixelRender_Z2I_555;

	GenericTriangleRender(v0,v1,v2);
}
#endif

#if 0
/*
 * Override smooth shaded 16 bit Z buffered texture mapped triangles
 */

void BR_ASM_CALL TestPixelRender_Z2TID(br_int_32 x, br_int_32 y)
{
	int i,u,v;
	SETUP_OFFSET;

	i = BrFixedToInt(CURRENT(pi) + 	dither[x & 3][y & 3]);
	u = BrFixedToInt((CURRENT(pu) >>8 )  + dither[(x+2) & 3][(y+2) & 3]);
	v = BrFixedToInt((CURRENT(pv) >>8 )  + dither[3-(x & 3)][3-(y & 3)]);

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = SHADE_LOOKUP(i,TEXTURE_LOOKUP(u,v));
	}
}

void BR_ASM_CALL TestPixelRender_Z2TI(br_int_32 x, br_int_32 y)
{
	int i;
	SETUP_OFFSET;

	i = BrFixedToInt(CURRENT(pi) + 	dither[x & 3][y & 3]);

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 =2 = CURRENT_UINT(pz);
		COLOUR_INDEX = SHADE_LOOKUP(i,
				TEXTURE_LOOKUP((CURRENT_INT(pu)>>8),(CURRENT_INT(pv)>>8)));
	}
}

void BR_ASM_CALL TriangleRenderPIZ2TI(struct temp_vertex *v0, struct temp_vertex *v1,struct temp_vertex *v2)
{
	zb.component_mask = CM_Z | CM_I | CM_U | CM_V | CM_Q ;
	zb.correct_mask = 0 ;

	zb.trapezoid_render = GenericTrapezoidRender;
	zb.pixel_render = ToggleFlag?TestPixelRender_Z2TI:TestPixelRender_Z2TID;

	GenericTriangleRender(v0,v1,v2);
}
#endif

#if 0
/*
 * Override 16 bit Z buffered texture mapped triangles
 */

void BR_ASM_CALL TestPixelRender_Z2TD(br_int_32 x, br_int_32 y)
{
	int u,v;
	SETUP_OFFSET;

	u = BrFixedToInt((CURRENT(pu) >>8 )  + dither[x & 3][y & 3]);
	v = BrFixedToInt((CURRENT(pv) >>8 )  + dither[x & 3][y & 3]);

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = TEXTURE_LOOKUP(u,v);
	}
}

void BR_ASM_CALL TestPixelRender_Z2T(br_int_32 x, br_int_32 y)
{
	SETUP_OFFSET;

	if(CURRENT_UINT(pz) < DEPTH_2) {
		DEPTH_2 = CURRENT_UINT(pz);
		COLOUR_INDEX = TEXTURE_LOOKUP((CURRENT_INT(pu)>>8),(CURRENT_INT(pv)>>8));
	}
}

void BR_ASM_CALL TriangleRenderPIZ2T(struct temp_vertex *v0, struct temp_vertex *v1,struct temp_vertex *v2)
{
	zb.component_mask = CM_Z |  CM_U | CM_V | CM_Q ;
	zb.correct_mask = 0 ;

	zb.trapezoid_render = GenericTrapezoidRender;
	
	zb.pixel_render = ToggleFlag?TestPixelRender_Z2T:TestPixelRender_Z2TD;

	GenericTriangleRender(v0,v1,v2);
}
#endif

