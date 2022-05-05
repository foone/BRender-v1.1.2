/*
 * $BC<"make -f 3ds2br.mak %s.obj;">
 *
 *    Copyright Voxar Limited 1995
 *
 *    parse3ds.c : Parses a blah.3ds binary file, attempts to build a data
 *                 structure corresponding to the objects described,
 *                 returning whether or not this was successful.
 *
 *  Comments are rather thin on the ground in this file. However, it is
 *  has ben cooded using a particularly idiomatic style, so once the 
 *  documentation has been read and understood, it should be fairly easy
 *  to follow the data as it trickes from the top of the stack through
 *  various data structres, until it is converted into a BRender equivalent
 *  and put aside.
 *
 *  Helpfully, all of the chunk names in the rather large switch statements
 *  appear in the same order as in the 3DStudio chunk documentation....
 *
 *    $Id: parse3ds.c 1.8 1995/08/31 16:39:18 sam Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <brender.h>
#include <fmt.h>

#include "basetype.h"
#include "bininstr.h"
#include "parse3ds.h"

#define SHOW_BROKEN_CHUNKS       (TRUE)
#define SHOW_KEYFRAME_DATA       (FALSE)
#define MAX_DISPLAY_DEPTH        (10)

#define SHOW_STACK				FALSE

#define TEST_WIREFRAME           (FALSE)

#define USE_MATERIAL_OPACITY     (FALSE)
#define USE_BR_MATF_TWO_SIDED    (FALSE)
#define USE_BR_MATF_DECAL        (FALSE)
#define USE_BR_MATF_PERSPECTIVE  (FALSE)

#define RED_GREYSCALE_FACTOR     (0.3)
#define GREEN_GREYSCALE_FACTOR   (0.6)
#define BLUE_GREYSCALE_FACTOR    (0.1)

#define FENCE                    (65535)

#define HALF_IMAGE_PLANE_MM      (21.2237408104802371)

typedef enum ChunkIdTag_t {

    /* The id tag for the mythic UNIDENTIFIED chunk doesnt matter, */
    /* so long as it is different from all of the other id tags */

    UNIDENTIFIED                                = 0x0000,

    COLOR_F                                     = 0x0010,
    COLOR_24                                    = 0x0011,
    INT_PERCENTAGE                              = 0x0030,
    FLOAT_PERCENTAGE                            = 0x0031,
    MAT_MAPNAME                                 = 0xA300,

        M3DMAGIC                                = 0x4D4D,
            M3D_VERSION                         = 0x0002,
            MDATA                               = 0x3D3D,
                MESH_VERSION                    = 0x3D3E,
                MASTER_SCALE                    = 0x0100,
                LO_SHADOW_BIAS                  = 0x1400,
                HI_SHADOW_BIAS                  = 0x1410,
                SHADOW_MAP_SIZE                 = 0x1420,
                SHADOW_SAMPLES                  = 0x1430,
                SHADOW_RANGE                    = 0x1440,
                SHADOW_FILTER                   = 0x1450,
                AMBIENT_LIGHT                   = 0x2100,
                O_CONSTS                        = 0x1500,
                BIT_MAP                         = 0x1100,
                SOLID_BGND                      = 0x1200,
                V_GRADIENT                      = 0x1300,
                USE_BIT_MAP                     = 0x1101,
                USE_SOLID_BGND                  = 0x1201,
                USE_V_GRADIENT                  = 0x1301,
                FOG                             = 0x2200,
                    FOG_BGND                    = 0x2210,
                DISTANCE_CUE                    = 0x2300,
                    DCUE_BGND                   = 0x2310,
                USE_FOG                         = 0x2201,
                USE_DISTANCE_CUE                = 0x2301,
                DEFAULT_VIEW                    = 0x3000,
                    VIEW_TOP                    = 0x3010,
                    VIEW_BOTTOM                 = 0x3020,
                    VIEW_LEFT                   = 0x3030,
                    VIEW_RIGHT                  = 0x3040,
                    VIEW_FRONT                  = 0x3050,
                    VIEW_BACK                   = 0x3060,
                    VIEW_USER                   = 0x3070,
                    VIEW_CAMERA                 = 0x3080,
                MAT_ENTRY                       = 0xAFFF,
                    MAT_NAME                    = 0xA000,
                    MAT_AMBIENT                 = 0xA010,
                    MAT_DIFFUSE                 = 0xA020,
                    MAT_SPECULAR                = 0xA030,
                    MAT_SHININESS               = 0xA040,
                    MAT_TRANSPARENCY            = 0xA050,
                    MAT_XPFALL                  = 0xA052,
                    MAT_USE_XPFALL              = 0xA240,
                    MAT_REFBLUR                 = 0xA053,
                    MAT_USE_REFBLUR             = 0xA250,
                    MAT_SELF_ILLUM              = 0xA080,
                    MAT_TWO_SIDE                = 0xA081,
                    MAT_DECAL                   = 0xA082,
                    MAT_ADDITIVE                = 0xA083,
                    MAT_SHADING                 = 0xA100,
                    MAT_TEXMAP                  = 0xA200,
                    MAT_SXP_TEXT_DATA           = 0xA320,
                    MAT_OPACMAP                 = 0xA210,
                    MAT_SXP_OPAC_DATA           = 0xA322,
                    MAT_REFLMAP                 = 0xA220,
                    MAT_ACUBIC                  = 0xA310,
                    MAT_BUMPMAP                 = 0xA230,
                    MAT_SXP_BUMP_DATA           = 0xA324,
                NAMED_OBJECT                    = 0x4000,
                    N_TRI_OBJECT                = 0x4100,
                        POINT_ARRAY             = 0x4110,
                        POINT_FLAG_ARRAY        = 0x4111,
                        FACE_ARRAY              = 0x4120,
                            MSH_MAT_GROUP       = 0x4130,
                            SMOOTH_GROUP        = 0x4150,
                        TEX_VERTS               = 0x4140,
                        MESH_MATRIX             = 0x4160,
                        MESH_TEXTURE_INFO       = 0x4170,
                        PROC_NAME               = 0x4181,
                        PROC_DATA               = 0x4182,
                    N_DIRECT_LIGHT              = 0x4600,
                       DL_OFF                   = 0x4620,
                       DL_SPOTLIGHT             = 0x4610,
                           DL_SHADOWED          = 0x4630,
                           DL_LOCAL_SHADOW2     = 0x4641,
                           DL_SEE_CONE          = 0x4650,
                    N_CAMERA                    = 0x4700,
                       CAM_SEE_CONE             = 0x4710,
                    OBJ_HIDDEN                  = 0x4010,
                    OBJ_VIS_LOFTER              = 0x4011,
                    OBJ_DOESNT_CAST             = 0x4012,
                    OBJ_MATTE                   = 0x4013,
                    OBJ_FAST                    = 0x4014,
                    OBJ_PROCEDURAL              = 0x4015,
                    OBJ_FROZEN                  = 0x4016,
        KFDATA                                  = 0xB000,
            KFHDR                               = 0xB00A,
            KFSEG                               = 0xB008,
            KFCURTIME                           = 0xB009,
            OBJECT_NODE_TAG                     = 0xB002,
            CAMERA_NODE_TAG                     = 0xB003,
            TARGET_NODE_TAG                     = 0xB004,
            LIGHT_NODE_TAG                      = 0xB005,
            SPOTLIGHT_NODE_TAG                  = 0xB007,
            L_TARGET_NODE_TAG                   = 0xB006,
                NODE_HDR                        = 0xB010,
                PIVOT                           = 0xB013,
                INSTANCE_NAME                   = 0xB011,
                MORPH_SMOOTH                    = 0xB015,
                BOUNDBOX                        = 0xB014,
                POS_TRACK_TAG                   = 0xB020,
                COL_TRACK_TAG                   = 0xB025,
                ROT_TRACK_TAG                   = 0xB021,
                SCL_TRACK_TAG                   = 0xB022,
                MORPH_TRACK_TAG                 = 0xB026,
                FOV_TRACK_TAG                   = 0xB023,
                ROLL_TRACK_TAG                  = 0xB024,
                HOT_TRACK_TAG                   = 0xB027,
                FALL_TRACK_TAG                  = 0xB028,

        MLIBMAGIC                               = 0x3DAA,

} ChunkIdTag_t;


typedef enum ChunkFlag_t {
    GOT_COLOR_F                                 = 0x00000001, /* generic    */
    GOT_COLOR_24                                = 0x00000002, /* generic    */
    GOT_INT_PERCENTAGE                          = 0x00000001, /* generic    */
    GOT_FLOAT_PERCENTAGE                        = 0x00000002, /* generic    */
    GOT_MAT_MAPNAME                             = 0x00000004, /* generic    */

        GOT_M3DMAGIC                            = 0x00000000, /* req, once  */
            GOT_M3D_VERSION                     = 0x00000001, /* req, once  */
            GOT_MDATA                           = 0x00000002, /* req, once  */
                GOT_MESH_VERSION                = 0x00000001, /* req, once  */
                GOT_MASTER_SCALE                = 0x00000002, /* req, once  */
                GOT_LO_SHADOW_BIAS              = 0x00000004, /* opt, once  */
                GOT_HI_SHADOW_BIAS              = 0x00000008, /* opt, once  */
                GOT_SHADOW_MAP_SIZE             = 0x00000010, /* opt, once  */
                GOT_SHADOW_SAMPLES              = 0x00000020, /* opt, once  */
                GOT_SHADOW_RANGE                = 0x00000040, /* opt, once  */
                GOT_SHADOW_FILTER               = 0x00000080, /* opt, once  */
                GOT_AMBIENT_LIGHT               = 0x00000100, /* opt, once  */
                    /* COLOR_<X>                                 req, once  */
                GOT_O_CONSTS                    = 0x00000200, /* opt, once  */
                GOT_BIT_MAP                     = 0x00000400, /* opt, once  */
                GOT_SOLID_BGND                  = 0x00000800, /* opt, once  */
                GOT_V_GRADIENT                  = 0x00001000, /* opt, once  */
                    /* COLOR_<X>                                 req, three */
                GOT_USE_BIT_MAP                 = 0x00002000, /* opt, once  */
                GOT_USE_SOLID_BGND              = 0x00004000, /* opt, once  */
                GOT_USE_V_GRADIENT              = 0x00008000, /* opt, once  */
                GOT_FOG                         = 0x00010000, /* opt, once  */
                    /* COLOR_<X>                                 req, once  */
                    GOT_FOG_BGND                = 0x00000000, /* opt, flag  */
                GOT_DISTANCE_CUE                = 0x00020000, /* opt, once  */
                    GOT_DCUE_BGND               = 0x00000000, /* opt, flag  */
                GOT_USE_FOG                     = 0x00040000, /* opt, once  */
                GOT_USE_DISTANCE_CUE            = 0x00080000, /* opt, once  */
                GOT_DEFAULT_VIEW                = 0x00100000, /* opt, once  */
                    GOT_VIEW_TOP                = 0x00000001, /* xopt, once */
                    GOT_VIEW_BOTTOM             = 0x00000002, /* xopt, once */
                    GOT_VIEW_LEFT               = 0x00000004, /* xopt, once */
                    GOT_VIEW_RIGHT              = 0x00000008, /* xopt, once */
                    GOT_VIEW_FRONT              = 0x00000010, /* xopt, once */
                    GOT_VIEW_BACK               = 0x00000020, /* xopt, once */
                    GOT_VIEW_USER               = 0x00000040, /* xopt, once */
                    GOT_VIEW_CAMERA             = 0x00000080, /* xopt, once */
                GOT_MAT_ENTRY                   = 0x00000000, /* opt, any   */
                    GOT_MAT_NAME                = 0x00000001, /* req, once  */
                    GOT_MAT_AMBIENT             = 0x00000002, /* req, once  */
                        /* COLOR_<X>                             req, once  */
                    GOT_MAT_DIFFUSE             = 0x00000004, /* req, once  */
                        /* COLOR_<X>                             req, once  */
                    GOT_MAT_SPECULAR            = 0x00000008, /* req, once  */
                        /* COLOR_<X>                             req, once  */
                    GOT_MAT_SHININESS           = 0x00000010, /* req, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                    GOT_MAT_TRANSPARENCY        = 0x00000020, /* req, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                    GOT_MAT_XPFALL              = 0x00000040, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                    GOT_MAT_USE_XPFALL          = 0x00000080, /* opt, once  */
                    GOT_MAT_REFBLUR             = 0x00000100, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                    GOT_MAT_USE_REFBLUR         = 0x00000200, /* opt, once  */
                    GOT_MAT_SELF_ILLUM          = 0x00000000, /* opt, flag  */
                    GOT_MAT_TWO_SIDE            = 0x00000400, /* opt, once  */
                    GOT_MAT_DECAL               = 0x00000800, /* opt, once  */
                    GOT_MAT_ADDITIVE            = 0x00000000, /* opt, flag  */
                    GOT_MAT_SHADING             = 0x00001000, /* req, once  */
                    GOT_MAT_TEXMAP              = 0x00002000, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                        /* MAT_MAPNAME                           req, once  */
                    GOT_MAT_SXP_TEXT_DATA       = 0x00004000, /* opt, once  */
                    GOT_MAT_OPACMAP             = 0x00008000, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                        /* MAT_MAPNAME                           req, once  */
                    GOT_MAT_SXP_OPAC_DATA       = 0x00010000, /* opt, once  */
                    GOT_MAT_REFLMAP             = 0x00020000, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                        /* MAT_MAPNAME                           req, once  */
                    GOT_MAT_ACUBIC              = 0x00040000, /* opt, once  */
                    GOT_MAT_BUMPMAP             = 0x00080000, /* opt, once  */
                        /* <X>_PERCENTAGE                        req, once  */
                        /* MAT_MAPNAME                           req, once  */
                    GOT_MAT_SXP_BUMP_DATA       = 0x00100000, /* opt, once  */
                GOT_NAMED_OBJECT                = 0x00000000, /* opt, any   */
                    GOT_N_TRI_OBJECT            = 0x00000001, /* xopt, once */
                        GOT_POINT_ARRAY         = 0x00000001, /* req, once  */
                        GOT_POINT_FLAG_ARRAY    = 0x00000002, /* opt, once  */
                        GOT_FACE_ARRAY          = 0x00000004, /* req, once  */
                            GOT_MSH_MAT_GROUP   = 0x00000000, /* opt, any   */
                            GOT_SMOOTH_GROUP    = 0x00000001, /* opt, once  */
                        GOT_TEX_VERTS           = 0x00000008, /* opt, once  */
                        GOT_MESH_MATRIX         = 0x00000010, /* opt, once  */
                        GOT_MESH_TEXTURE_INFO   = 0x00000020, /* opt, once  */
                        GOT_PROC_NAME           = 0x00000040, /* opt, once  */
                        GOT_PROC_DATA           = 0x00000080, /* opt, once  */
                    GOT_N_DIRECT_LIGHT          = 0x00000002, /* xopt, once */
                       /* COLOR_<X>                              req, once  */
                       GOT_DL_OFF               = 0x00000001, /* opt, once  */
                       GOT_DL_SPOTLIGHT         = 0x00000002, /* opt, once  */
                           GOT_DL_SHADOWED      = 0x00000001, /* opt, once  */
                           GOT_DL_LOCAL_SHADOW2 = 0x00000002, /* opt, once  */
                           GOT_DL_SEE_CONE      = 0x00000004, /* opt, once  */
                    GOT_N_CAMERA                = 0x00000004, /* xopt, once */
                       GOT_CAM_SEE_CONE         = 0x00000001, /* opt, once  */
                    GOT_OBJ_HIDDEN              = 0x00000002, /* opt, once  */
                    GOT_OBJ_VIS_LOFTER          = 0x00000004, /* opt, once  */
                    GOT_OBJ_DOESNT_CAST         = 0x00000008, /* opt, once  */
                    GOT_OBJ_MATTE               = 0x00000010, /* opt, once  */
                    GOT_OBJ_FAST                = 0x00000020, /* opt, once  */
                    GOT_OBJ_PROCEDURAL          = 0x00000040, /* opt, once  */
                    GOT_OBJ_FROZEN              = 0x00000080, /* opt, once  */
        GOT_KFDATA                              = 0x00000004, /* opt, once  */
            GOT_KFHDR                           = 0x00000001, /* req, once  */
            GOT_KFSEG                           = 0x00000002, /* req, once  */
            GOT_KFCURTIME                       = 0x00000004, /* req, once  */
            GOT_OBJECT_NODE_TAG                 = 0x00000000, /* opt, any   */
            GOT_CAMERA_NODE_TAG                 = 0x00000000, /* opt, any   */
            GOT_TARGET_NODE_TAG                 = 0x00000000, /* opt, any   */
            GOT_LIGHT_NODE_TAG                  = 0x00000000, /* opt, any   */
            GOT_SPOTLIGHT_NODE_TAG              = 0x00000000, /* opt, any   */
            GOT_L_TARGET_NODE_TAG               = 0x00000000, /* opt, any   */
                GOT_NODE_HDR                    = 0x00000001, /* req, once  */
                GOT_PIVOT                       = 0x00000002, /* req, once  */
                GOT_INSTANCE_NAME               = 0x00000004, /* opt, once  */
                GOT_MORPH_SMOOTH                = 0x00000008, /* opt, once  */
                GOT_BOUNDBOX                    = 0x00000010, /* req, once  */
                GOT_POS_TRACK_TAG               = 0x00000020, /* req, once  */
                GOT_COL_TRACK_TAG               = 0x00000040, /* req, once  */
                GOT_ROT_TRACK_TAG               = 0x00000080, /* req, once  */
                GOT_SCL_TRACK_TAG               = 0x00000100, /* req, once  */
                GOT_MORPH_TRACK_TAG             = 0x00000200, /* opt, once  */
                GOT_FOV_TRACK_TAG               = 0x00000400, /* req, once  */
                GOT_ROLL_TRACK_TAG              = 0x00000800, /* req, once  */
                GOT_HOT_TRACK_TAG               = 0x00001000, /* req, once  */
                GOT_FALL_TRACK_TAG              = 0x00002000  /* req, once  */

} ChunkFlag_t;

typedef struct Color_t {
    Float_t red;
    Float_t green;
    Float_t blue;
} Color_t;


typedef enum MatShading_t {
    WIREFRAME,
    FLAT,
    GOURAUD
} MatShading_t;


typedef struct PixmapList_t {
    br_pixelmap         *pixelmap;
    struct PixmapList_t *next;
} PixmapList_t;

typedef struct PixmapRef_t {
    Bool_t  is_reflection_map;
    char    *mat_mapname;
    Float_t strength;
} PixmapRef_t;


typedef struct MatEntry_t {
    char         *mat_name;
    Color_t      mat_ambient;
    Color_t      mat_diffuse;
    Color_t      mat_specular;
    Float_t      mat_transparency;
    Float_t      mat_shininess;
    MatShading_t mat_shading;
    Bool_t       mat_two_side;
    Bool_t       mat_decal;
    PixmapRef_t  pixmap_ref;
} MatEntry_t;


typedef struct MaterialList_t {
    MatShading_t          mat_shading;
    br_material           *material;
    struct MaterialList_t *next;
} MaterialList_t;


typedef struct PointArray_t {
    br_uint_16 n_vertices;
    br_vector3 *vertices;
} PointArray_t;


typedef struct Face_t {
    br_uint_16 vertices[3];
    br_uint_16 flags;
} Face_t;


typedef struct MshMatGroup_t {
    MaterialList_t *material_link;
    br_uint_16     n_indexes;
    br_uint_16     *indexes;
    struct MshMatGroup_t *next;
} MshMatGroup_t;


typedef struct FaceArray_t {
    br_uint_16    n_faces;
    Face_t        *faces;
    MshMatGroup_t *msh_mat_groups;
    br_uint_16    *smooth_group;
} FaceArray_t;


typedef struct TexVerts_t {
    br_uint_16  n_texverts;
    br_vector2  *texverts;
} TexVerts_t;


typedef struct NTriObj_t {
    PointArray_t point_array;
    FaceArray_t  face_array;
    TexVerts_t   tex_verts;
    br_matrix34  mesh_matrix;
} NTriObj_t;


typedef struct DlSpotlight_t {
    br_vector3 target;
    Float_t    cone_inner;
    Float_t    cone_outer;
} DlSpotlight_t;


typedef struct NDLight_t {
    br_vector3    posn;
    Color_t       color;
    Bool_t        is_off;
    Bool_t        is_spotlight;
    DlSpotlight_t dl_spotlight;
} NDLight_t;


typedef struct NCamera_t {
    br_vector3 posn;
    br_vector3 target;
    Float_t    bank_angle;
    Float_t    focal_length;
} NCamera_t;


typedef struct Light_t {
    br_uint_8 type;
    br_colour colour;
    br_angle cone_outer;
    br_angle cone_inner;
    Bool_t is_off;
} Light_t;


typedef struct Camera_t {
    br_angle field_of_view;
} Camera_t;


typedef struct Model_t {
    br_model    *fill_model;
    br_model    *wire_model;
} Model_t;


typedef enum NamedObjType_t {
    NONE,
    LIGHT,
    CAMERA,
    MODEL
} NamedObjType_t;


typedef struct NamedObj_t {
    char              *name;
    NamedObjType_t    type;
    union {
        Light_t       *light;
        Model_t       *model;
        Camera_t      *camera;
    } data;
    br_matrix34 to_world;
    br_matrix34 from_world;
    struct NamedObj_t *next;
} NamedObj_t;


typedef struct NodeHdr_t {
    Int_t      index;
    Int_t      parent;
    NamedObj_t *named_obj;
} NodeHdr_t;


typedef struct NodeTag_t {
    NodeHdr_t  node_hdr;
    char       *instance_name;
    Bool_t     has_pivot;
    br_vector3 pivot;
    Bool_t     has_boundbox;
    br_bounds  boundbox;
    struct NodeTag_t *next;
} NodeTag_t;


typedef struct Stack_t {
    ChunkIdTag_t id_tag;
    br_uint_32   length;
    br_uint_32   done;
    ChunkFlag_t  flags;
    union { Color_t        color;
            Float_t        percent;
            char           *string;
            MatShading_t   mat_shading;
            PixmapRef_t    pixmap_ref;
            MatEntry_t     mat_entry;
            PointArray_t   point_array;
            FaceArray_t    face_array;
            br_uint_16     *smooth_group;
            MshMatGroup_t  *msh_mat_group;
            TexVerts_t     tex_verts;
            br_matrix34    mesh_matrix;
            NTriObj_t      n_tri_obj;
            DlSpotlight_t  dl_spotlight;
            NDLight_t      n_d_light;
            NCamera_t      n_camera;
            NamedObj_t     *named_obj;
            NodeHdr_t      node_hdr;
            br_vector3     pivot;
            br_bounds      boundbox;
            NodeTag_t      *node_tag;
          } data;
} Stack_t;


typedef enum State_t {
    OK,
    PARSE_ERROR,
    OUT_OF_MEMORY
} State_t;


#if SHOW_BROKEN_CHUNKS

typedef struct TagNames_t {
    ChunkIdTag_t id_tag;
    char *name;
} TagNames_t;

static TagNames_t tag_names[] = {
    {COLOR_F, "COLOR_F"},
    {COLOR_24, "COLOR_24"},
    {INT_PERCENTAGE, "INT_PERCENTAGE"},
    {FLOAT_PERCENTAGE, "FLOAT_PERCENTAGE"},
    {MAT_MAPNAME, "MAT_MAPNAME"},
    {M3DMAGIC, "M3DMAGIC"},
    {M3D_VERSION, "M3D_VERSION"},
    {MDATA, "MDATA"},
    {MESH_VERSION, "MESH_VERSION"},
    {MASTER_SCALE, "MASTER_SCALE"},
    {LO_SHADOW_BIAS, "LO_SHADOW_BIAS"},
    {HI_SHADOW_BIAS, "HI_SHADOW_BIAS"},
    {SHADOW_MAP_SIZE, "SHADOW_MAP_SIZE"},
    {SHADOW_SAMPLES, "SHADOW_SAMPLES"},
    {SHADOW_RANGE, "SHADOW_RANGE"},
    {SHADOW_FILTER, "SHADOW_FILTER"},
    {AMBIENT_LIGHT, "AMBIENT_LIGHT"},
    {O_CONSTS, "O_CONSTS"},
    {BIT_MAP, "BIT_MAP"},
    {SOLID_BGND, "SOLID_BGND"},
    {V_GRADIENT, "V_GRADIENT"},
    {USE_BIT_MAP, "USE_BIT_MAP"},
    {USE_SOLID_BGND, "USE_SOLID_BGND"},
    {USE_V_GRADIENT, "USE_V_GRADIENT"},
    {FOG, "FOG"},
    {FOG_BGND, "FOG_BGND"},
    {DISTANCE_CUE, "DISTANCE_CUE"},
    {DCUE_BGND, "DCUE_BGND"},
    {USE_FOG, "USE_FOG"},
    {USE_DISTANCE_CUE, "USE_DISTANCE_CUE"},
    {DEFAULT_VIEW, "DEFAULT_VIEW"},
    {VIEW_TOP, "VIEW_TOP"},
    {VIEW_BOTTOM, "VIEW_BOTTOM"},
    {VIEW_LEFT, "VIEW_LEFT"},
    {VIEW_RIGHT, "VIEW_RIGHT"},
    {VIEW_FRONT, "VIEW_FRONT"},
    {VIEW_BACK, "VIEW_BACK"},
    {VIEW_USER, "VIEW_USER"},
    {VIEW_CAMERA, "VIEW_CAMERA"},
    {MAT_ENTRY, "MAT_ENTRY"},
    {MAT_NAME, "MAT_NAME"},
    {MAT_AMBIENT, "MAT_AMBIENT"},
    {MAT_DIFFUSE, "MAT_DIFFUSE"},
    {MAT_SPECULAR, "MAT_SPECULAR"},
    {MAT_SHININESS, "MAT_SHININESS"},
    {MAT_TRANSPARENCY, "MAT_TRANSPARENCY"},
    {MAT_XPFALL, "MAT_XPFALL"},
    {MAT_USE_XPFALL, "MAT_USE_XPFALL"},
    {MAT_REFBLUR, "MAT_REFBLUR"},
    {MAT_USE_REFBLUR, "MAT_USE_REFBLUR"},
    {MAT_SELF_ILLUM, "MAT_SELF_ILLUM"},
    {MAT_TWO_SIDE, "MAT_TWO_SIDE"},
    {MAT_DECAL, "MAT_DECAL"},
    {MAT_ADDITIVE, "MAT_ADDITIVE"},
    {MAT_SHADING, "MAT_SHADING"},
    {MAT_TEXMAP, "MAT_TEXMAP"},
    {MAT_SXP_TEXT_DATA, "MAT_SXP_TEXT_DATA"},
    {MAT_OPACMAP, "MAT_OPACMAP"},
    {MAT_SXP_OPAC_DATA, "MAT_SXP_OPAC_DATA"},
    {MAT_REFLMAP, "MAT_REFLMAP"},
    {MAT_ACUBIC, "MAT_ACUBIC"},
    {MAT_BUMPMAP, "MAT_BUMPMAP"},
    {MAT_SXP_BUMP_DATA, "MAT_SXP_BUMP_DATA"},
    {NAMED_OBJECT, "NAMED_OBJECT"},
    {N_TRI_OBJECT, "N_TRI_OBJECT"},
    {POINT_ARRAY, "POINT_ARRAY"},
    {POINT_FLAG_ARRAY, "POINT_FLAG_ARRAY"},
    {FACE_ARRAY, "FACE_ARRAY"},
    {MSH_MAT_GROUP, "MSH_MAT_GROUP"},
    {SMOOTH_GROUP, "SMOOTH_GROUP"},
    {TEX_VERTS, "TEX_VERTS"},
    {MESH_MATRIX, "MESH_MATRIX"},
    {MESH_TEXTURE_INFO, "MESH_TEXTURE_INFO"},
    {PROC_NAME, "PROC_NAME"},
    {PROC_DATA, "PROC_DATA"},
    {N_DIRECT_LIGHT, "N_DIRECT_LIGHT"},
    {DL_OFF, "DL_OFF"},
    {DL_SPOTLIGHT, "DL_SPOTLIGHT"},
    {DL_SHADOWED, "DL_SHADOWED"},
    {DL_LOCAL_SHADOW2, "DL_LOCAL_SHADOW2"},
    {DL_SEE_CONE, "DL_SEE_CONE"},
    {N_CAMERA, "N_CAMERA"},
    {CAM_SEE_CONE, "CAM_SEE_CONE"},
    {OBJ_HIDDEN, "OBJ_HIDDEN"},
    {OBJ_VIS_LOFTER, "OBJ_VIS_LOFTER"},
    {OBJ_DOESNT_CAST, "OBJ_DOESNT_CAST"},
    {OBJ_MATTE, "OBJ_MATTE"},
    {OBJ_FAST, "OBJ_FAST"},
    {OBJ_PROCEDURAL, "OBJ_PROCEDURAL"},
    {OBJ_FROZEN, "OBJ_FROZEN"},
    {KFDATA, "KFDATA"},
    {KFHDR, "KFHDR"},
    {KFSEG, "KFSEG"},
    {KFCURTIME, "KFCURTIME"},
    {OBJECT_NODE_TAG, "OBJECT_NODE_TAG"},
    {CAMERA_NODE_TAG, "CAMERA_NODE_TAG"},
    {TARGET_NODE_TAG, "TARGET_NODE_TAG"},
    {LIGHT_NODE_TAG, "LIGHT_NODE_TAG"},
    {SPOTLIGHT_NODE_TAG, "SPOTLIGHT_NODE_TAG"},
    {L_TARGET_NODE_TAG, "L_TARGET_NODE_TAG"},
    {NODE_HDR, "NODE_HDR"},
    {PIVOT, "PIVOT"},
    {INSTANCE_NAME, "INSTANCE_NAME"},
    {MORPH_SMOOTH, "MORPH_SMOOTH"},
    {BOUNDBOX, "BOUNDBOX"},
    {POS_TRACK_TAG, "POS_TRACK_TAG"},
    {COL_TRACK_TAG, "COL_TRACK_TAG"},
    {ROT_TRACK_TAG, "ROT_TRACK_TAG"},
    {SCL_TRACK_TAG, "SCL_TRACK_TAG"},
    {MORPH_TRACK_TAG, "MORPH_TRACK_TAG"},
    {FOV_TRACK_TAG, "FOV_TRACK_TAG"},
    {ROLL_TRACK_TAG, "ROLL_TRACK_TAG"},
    {HOT_TRACK_TAG, "HOT_TRACK_TAG"},
    {FALL_TRACK_TAG, "FALL_TRACK_TAG"},
    {MLIBMAGIC, "MLIBMAGIC"},
    {UNIDENTIFIED, "Unidentified"}       /* this must be the last entry */
};

static
    void
DisplayTop(
    char    *operation,
    Stack_t *top,
    Stack_t *base
) {
    Int_t i,j;

    ASSERT_TRAP(operation != NULL && top != NULL);

    i = 0;
    while (tag_names[i].id_tag != UNIDENTIFIED
             && tag_names[i].id_tag != top->id_tag) {
       i += 1;
    }

	for(j=0; j < top-base; j++)
		fputs("    ",stdout);

    fprintf(stdout,"%s: %s chunk (0x%04x, %d/%d)\n",
            operation,tag_names[i].name,
            top->id_tag,top->done,top->length);
}

#endif

static
    void
BlockCopy(
    void  *dest,
    void  *src,
    Int_t n_bytes
) {
    Int_t i;
    char  *dest_bp;
    char  *src_bp;

    ASSERT_TRAP(dest != NULL && src != NULL);

    dest_bp = (char*)(dest);
    src_bp  = (char*)(src);

    for (i=0; i<n_bytes; i++) {
         *dest_bp++ = *src_bp++;
    }
}


static                        /* Skips all of the remaining bytes in the  */
    Bool_t                    /* chunk currently at the top of the stack. */
SkipRest(
    BinInStream_tp stream,
    Stack_t        *top
) {
    ASSERT_TRAP(stream != NULL && top != NULL);

    if(SkipBytes(stream,top->length - top->done)) {
        top->done = top->length;
        return TRUE;
    }
    return FALSE;
}


static                        /* Reads a chunk header from the input stream */
    Bool_t                    /* into the top of the stack.                 */
ReadHeader(
    BinInStream_tp stream,
    Stack_t        *top
) {
    br_uint_16 id_tag;
    br_uint_32 length;

    ASSERT_TRAP(stream != NULL && top != NULL);

    if (ReadUInt16(stream,&id_tag)
          && ReadUInt32(stream,&length)) {
        top->id_tag = (ChunkIdTag_t)(id_tag);
        top->length = length;
        top->done   = sizeof(br_uint_16) + sizeof(br_uint_32);
        top->flags  = 0;

        return TRUE;
    }
    return FALSE;
}

static                        /* Read a null terminated string of length */
    Bool_t                    /* less than "max_n_bytes" from the input  */
ReadString(                   /* stream, into "buffer".                  */
    BinInStream_tp stream,
    Stack_t        *top,
    Int_t          max_n_bytes,
    char           *buffer
) {
    Int_t i;

    ASSERT_TRAP(stream != NULL && top != NULL);

    for (i=0; (i < max_n_bytes) && (top->done < top->length); i++) {
        if (ReadChar(stream,buffer+i)) {
            top->done += sizeof(char);
            if (buffer[i] == '\0') {
                return TRUE;
            }
        } else {
            return FALSE;
        }
    }
    return FALSE;
}


static                        /* Read an (R,G,B) triple of floating point */
    Bool_t                    /* values into the top of the stack.        */
ReadColorF(
    BinInStream_tp stream,
    Stack_t        *top
) {
    Color_t *color;

    ASSERT_TRAP(stream != NULL && top != NULL);

    color = &(top->data.color);

    if (ReadFloat(stream,&(color->red))
          && ReadFloat(stream,&(color->green))
          && ReadFloat(stream,&(color->blue))
          && (color->red   >= 0.0 && color->red   <= 1.0)
          && (color->green >= 0.0 && color->green <= 1.0)
          && (color->blue  >= 0.0 && color->blue  <= 1.0)) {
        top->done += 3 * sizeof(Float_t);

        return TRUE;
    }
    return FALSE;
}

static                       /* Read an (R,G,B) triple of unsigned bytes, */
    Bool_t                   /* convert them into floating point values,  */
ReadColor24(                 /* copy the converted triple into the top of */
    BinInStream_tp stream,   /* the stack.                                */
    Stack_t        *top
) {
    Color_t *color;
    br_uint_8 red,green,blue;

    ASSERT_TRAP(stream != NULL && top != NULL);

    color = &(top->data.color);

    if(ReadUInt8(stream,&red)
          && ReadUInt8(stream,&green)
          && ReadUInt8(stream,&blue)) {
        color->red   = ((Float_t)(red))   / 255.0;
        color->green = ((Float_t)(green)) / 255.0;
        color->blue  = ((Float_t)(blue))  / 255.0;

        top->done += 3 * sizeof(br_uint_8);

        return TRUE;
    }
    return FALSE;
}


static                  /* Convert a colour from the form given by Color_t */
    br_colour           /* into its BRender equivalent.                    */
ConvertColor(
    Color_t *color
) {
    return BR_COLOUR_RGB((float)floor(255.0 * color->red),
                         (float)floor(255.0 * color->green),
                         (float)floor(255.0 * color->blue));
}


static                  /* Take a colour in the form given by Color_t, */
    br_ufraction        /* and return its greyscale intensity.         */
GreyscaleOfColor(
    Color_t *color
) {
    return BrScalarToUFraction(
               BrFloatToScalar(
                   (float)((color->red   * RED_GREYSCALE_FACTOR)
                          +(color->green * GREEN_GREYSCALE_FACTOR)
                          +(color->blue  * BLUE_GREYSCALE_FACTOR))));
}


static                      /* Read an (X,Y,Z) triple from the input stream, */
    Bool_t                  /* convert it into a br_vector3, remembering     */
ReadPoint(                  /* to reflect in the planes y=z and z=0.         */
    BinInStream_tp stream,
    Stack_t        *top,
    br_vector3     *vector,
    Parse3dsBinOptions_t *options
) {
    Int_t   i;
    Float_t coords[3];

    ASSERT_TRAP(stream != NULL && top != NULL && vector != NULL);

    for (i=0; i<3; i++) {
        if (ReadFloat(stream,&coords[i])) {
            top->done += sizeof(Float_t);
        } else {
            return FALSE;
        }
    }

	if(options->correct_axis) {
	    /* Now convert to right-handed coord space properly. */

	    vector->v[0] = BrFloatToScalar((float)(coords[0]));
	    vector->v[1] = BrFloatToScalar((float)(coords[2]));
    	vector->v[2] = BrFloatToScalar((float)(-coords[1]));
	} else {
	    vector->v[0] = BrFloatToScalar((float)(coords[0]));
    	vector->v[1] = BrFloatToScalar((float)(coords[1]));
	    vector->v[2] = BrFloatToScalar((float)(coords[2]));
	}

    return TRUE;
}


static                      /* Initialise a pixmap reference, so that it */
    void                    /* does not refer to either a texture map or */
InitialisePixmapRef(        /* a reflection map.                         */
    PixmapRef_t *pixmap_ref
) {
    ASSERT_TRAP(pixmap_ref != NULL);

    pixmap_ref->mat_mapname       = NULL;
    pixmap_ref->is_reflection_map = FALSE;
}


static                      /* Deallocate the memory used by the name of */
    void                    /* any pixelmap referenced by the structure. */
DismantlePixmapRef(
    PixmapRef_t *pixmap_ref
) {
    ASSERT_TRAP(pixmap_ref != NULL);

    pixmap_ref->is_reflection_map = FALSE;
    if (pixmap_ref->mat_mapname != NULL) {
        BrMemFree(pixmap_ref->mat_mapname);
        pixmap_ref->mat_mapname = NULL;
    }
}


static                       /* If there is no existing pixelmap referred to */
    void                     /* by the structure, or one with a higher       */
UpdatePixmapRef(             /* strength has been found, make the structure  */
    PixmapRef_t  *exist_ref, /* refer to the newly found pixelmap.           */
    PixmapRef_t  *new_ref
) {
    ASSERT_TRAP(exist_ref != NULL && new_ref != NULL);

    if (exist_ref->mat_mapname == NULL
            || new_ref->strength > exist_ref->strength) {
        DismantlePixmapRef(exist_ref);
        BlockCopy(exist_ref,new_ref,sizeof(PixmapRef_t));
    } else {
        DismantlePixmapRef(new_ref);
    }
}

/*
 * This function allocates a new br_pixelmap, given its name. These
 * are used in textured materals. One holds the actual texture and one
 * the shade table.
 *
 * This function doesn't allocate any pixels. It just makes a dummy
 * br_pixelmap with the right identifier string.
 *
 * Each map that gets created this way gets threaded into a list, a
 * pointer to which is passed as the "maps" parameter.
 */

static
    br_pixelmap *
GetPixelmap(
    char         *mat_mapname,
    PixmapList_t **pixmap_list
) {
    char         *identifier;
    char         *suffix;
    br_pixelmap  *pixelmap;
    PixmapList_t *pixmap_link;

    ASSERT_TRAP(mat_mapname != NULL && pixmap_list != NULL);

    /* Make a copy of the name, without its dot extension */

    suffix = strrchr(mat_mapname,'.');

    if (suffix != NULL) {
        *suffix = '\0';
    }
    identifier = (char*)BrMemAllocate((strlen(mat_mapname)+1) * sizeof(char),
                                      BR_MEMORY_APPLICATION);
    if (identifier == NULL) {
        if (suffix != NULL) {
            *suffix = '.';
        }
        return NULL;
    }
    strcpy(identifier,mat_mapname);

    if (suffix != NULL) {
        *suffix = '.';
    }
    /* If we already have a map by the same name, share it */

    for (pixmap_link = *pixmap_list; pixmap_link != NULL;
                                          pixmap_link = pixmap_link->next) {
        if (strcmp (identifier, pixmap_link->pixelmap->identifier) == 0) {
            BrMemFree(identifier);
            return pixmap_link->pixelmap;
        }
    }
    /* Else make one */

    pixelmap = BrPixelmapAllocate(BR_PMT_INDEX_8,1,1,NULL,0);

    if (pixelmap == NULL) {
         BrMemFree(identifier);
         return NULL;
    }
    pixmap_link = (PixmapList_t*) BrMemAllocate(sizeof(PixmapList_t),
                                           BR_MEMORY_APPLICATION);

    if (pixmap_link == NULL) {
        BrPixelmapFree(pixelmap);
        BrMemFree(identifier);
        return NULL;
    }
    pixelmap->identifier = identifier;

    /* Thread the map into the list at the head */

    pixmap_link->pixelmap = pixelmap;
    pixmap_link->next     = *pixmap_list;
    *pixmap_list          = pixmap_link;

    return pixelmap;
}

static                         /* Deallocate all of the memory used by the */
    void                       /* list of pixelmaps.                       */
DeallocatePixmapList(
    PixmapList_t *pixmap_list
) {
    PixmapList_t *dud;

    while (pixmap_list != NULL) {
        dud = pixmap_list;
        pixmap_list = pixmap_list->next;

        BrPixelmapFree(dud->pixelmap);
        BrMemFree(dud);
    }
}

static                         /* Initialise a material so that all of its */
    void                       /* optional properties are turned off.      */
InitialiseMatEntry(
    MatEntry_t *mat_entry
) {
    ASSERT_TRAP(mat_entry != NULL);

    mat_entry->mat_name     = NULL;
    mat_entry->mat_two_side = FALSE;
    mat_entry->mat_decal    = FALSE;
	mat_entry->mat_ambient.red	 = 0.0;
	mat_entry->mat_ambient.green = 0.0;
	mat_entry->mat_ambient.blue  = 0.0;
	mat_entry->mat_diffuse.red	 = 0.75;
	mat_entry->mat_diffuse.green = 0.75;
	mat_entry->mat_diffuse.blue  = 0.75;
	mat_entry->mat_specular.red	 = 0.0;
	mat_entry->mat_specular.green = 0.0;
	mat_entry->mat_specular.blue  = 0.0;
	mat_entry->mat_transparency  = 0.0;
	mat_entry->mat_shininess  = 0.0;
	mat_entry->mat_shading  = FLAT;
    InitialisePixmapRef(&(mat_entry->pixmap_ref));

}

static                         /* Deallocate the memory used by the name of */
    void                       /* the material, and any pixelmap it may     */
DismantleMatEntry(             /* refer to.                                 */
    MatEntry_t *mat_entry
) {
    ASSERT_TRAP(mat_entry != NULL);

    if (mat_entry->mat_name != NULL) {
        BrMemFree(mat_entry->mat_name);
        mat_entry->mat_name = NULL;
    }
    DismantlePixmapRef(&(mat_entry->pixmap_ref));
}

/*
 * Convert a 3DS material to an equivalent BRender material.
 *
 * This function builds a BRender material from the information
 * contained in a 3DS material, and adds it to the registry
 *
 * Each material that is converted is threaded into a list.
 */

static
    MaterialList_t *
ConvertMaterial(
    MatEntry_t           *mat_entry,
    PixmapList_t         **pixmap_list,
    Parse3dsBinOptions_t *options       /* user options affect materials */
) {
    MaterialList_t *material_link;
    br_material    *material;
    br_pixelmap    *colour_map;
    br_pixelmap    *index_shade;

    ASSERT_TRAP(mat_entry != NULL && pixmap_list != NULL && options != NULL);

    material_link = (MaterialList_t*)BrMemAllocate(sizeof(MaterialList_t),
                                                   BR_MEMORY_APPLICATION);

    if (material_link == NULL) {
        return NULL;
    }
    material_link->mat_shading  = mat_entry->mat_shading;
    material_link->next         = NULL;

    material = BrMaterialAllocate(mat_entry->mat_name);

    if (material == NULL) {
        BrMemFree(material_link);
        return NULL;
    }
    /* Set the RGB true colour. This will only be used when rendering */
    /* 24-bit, but a clever application could use this to generate its */
    /* colour ramps */

    /* As per Sam's instructions, the diffuse colour is used as the */
    /* material's colour. The Ambient and specular colours only affect */
    /* Ka and Ks. */

    material->colour = ConvertColor(&(mat_entry->mat_diffuse));

    /* The Ambient, Diffuse and Specular components are used to do */
    /* lighting in all cases */

    material->ka = GreyscaleOfColor(&(mat_entry->mat_ambient));
    material->kd = GreyscaleOfColor(&(mat_entry->mat_diffuse));
    material->ks = GreyscaleOfColor(&(mat_entry->mat_specular));

#if USE_MATERIAL_OPACITY
    /* We have discovered that this causes the material to be rejected */

    material->opacity
         = (br_uint_8)floor(255.0 * mat_entry->mat_transparency);
#endif

    /* Set the "sharpness" of the specular highlight. Normally shows */
    /* up in the lighting equation as cos(theta)^power */

    material->power
         = BrFloatToScalar((float)(1.0 + (99.0 * mat_entry->mat_shininess)));

    /* Set the material flags. Most are direct BRender equivalents of */
    /* 3DS flags, except for perspective, which is a user choice. */

    material->flags = BR_MATF_LIGHT;

    if (mat_entry->mat_shading == GOURAUD) {
        material->flags |= BR_MATF_GOURAUD;
    }

#if USE_BR_MATF_PERSPECTIVE
    if (options->perspective_tex) {
        material->flags |= BR_MATF_PERSPECTIVE;
    }
#endif

#if USE_BR_MATF_TWO_SIDED
    /* This crashes the renderer when set */

    if (mat_entry->mat_two_side) {
        material->flags |= BR_MATF_TWO_SIDED;
    }
#endif

#if USE_BR_MATF_DECAL
    /* This is said to be unreliable */

    if (mat_entry->mat_decal) {
        material->flags |= BR_MATF_DECAL;
    }
#endif


    if (mat_entry->pixmap_ref.mat_mapname == NULL) {  /* Untextured material */

        /* HACK, grey shades tend to come out */

        material->index_base = 0;
        material->index_range = 63;

        material->colour_map = NULL;
        material->index_shade = NULL;

    } else {                                     /* Textured material */

        material->flags |= BR_MATF_MAP_COLOUR;

        if (mat_entry->pixmap_ref.is_reflection_map) {
            material->flags |= BR_MATF_ENVIRONMENT_I;

        }
        /* Set the texture map and shade table of the material (yes */
        /* they are oddly named). */

        /* This does not mean we load any pixel data. All we do is */
        /* allocate two br_pixelmap objects, one for the texture and */
        /* one for the shade table, and set the identifier strings to */
        /* the material name. We leave the pixels pointer NULL in */
        /* both. The registry is supposed to match these up by name */
        /* when the materials are loaded */

        /* The shade table name should be named "shade", Sam said. */

        colour_map = GetPixelmap(mat_entry->pixmap_ref.mat_mapname,
                                 pixmap_list);
        index_shade = GetPixelmap("shade",pixmap_list);

        if ((colour_map != NULL) && (index_shade != NULL)) {
            material->colour_map = colour_map;
            material->index_shade = index_shade;

        } else {
            BrMaterialFree(material);
            BrMemFree(material_link);
            return NULL;
        }
        material->index_base = 64;
        material->index_range = 31;

    }
    material_link->material = BrMaterialAdd(material);

    if (material_link->material == NULL) {
        BrMaterialFree(material);
        BrMemFree(material_link);
        return NULL;
    }

    if (options->log != NULL) {
        fprintf(options->log,"Material: \"%s\"\n", material->identifier);
        fprintf(options->log,"  colour:   %3d %3d %3d\n",
                BR_RED(material->colour),
                BR_GRN(material->colour),
                BR_BLU(material->colour));
        fprintf(options->log,"  ka kd ks: %f %f %f\n",
                BrScalarToFloat(material->ka),
                BrScalarToFloat(material->kd),
                BrScalarToFloat(material->ks));
        fprintf(options->log,"  index_base:     %3d\n", material->index_base);
        fprintf(options->log,"  index_range:    %3d\n", material->index_range);

        if (mat_entry->pixmap_ref.mat_mapname != NULL) {
            fprintf(options->log,"  texture:  \"%s\"\n",
                    colour_map->identifier);
            fprintf(options->log,"  shade:    \"%s\"\n",
                    index_shade->identifier);
        }
    }

    return material_link;
}

static                     /* Given a list of properly converted materials, */
    MaterialList_t *       /* attempt to find one with a specific name.     */
LookupMaterial(
    char           *name,
    MaterialList_t *list
) {
    MaterialList_t *item;

    for (item = list; item != NULL; item = item->next) {
         if (strcmp(name,item->material->identifier) == 0) {
             return item;
         }
    }
    return NULL;
}

static                     /* Return TRUE if the material specifies filled */
    Bool_t                 /* faces, and FALSE otherwise.                  */
FillMat(
    MaterialList_t *material_link
) {
    return (material_link == NULL || material_link->mat_shading != WIREFRAME);
}

static                     /* Return TRUE if the material specifies  */
    Bool_t                 /* wireframed faces, and FALSE otherwise. */
WireMat(
    MaterialList_t *material_link
) {
    return (material_link != NULL && material_link->mat_shading == WIREFRAME);
}


static                     /* Deallocate the memory used by the list of */
    void                   /* properly converted materials.             */
DeallocateMaterialList(
    MaterialList_t *material_list
) {
    MaterialList_t *dud;

    while (material_list != NULL) {
        dud = material_list;
        material_list = material_list->next;

        BrMaterialFree(dud->material);
        BrMemFree(dud);
    }
}


static                          /* Initialise a new point array to not yet */
    void                        /* contain any information about the       */
InitialisePointArray(           /* vertices shortly to be read from the    */
    PointArray_t *point_array   /* input stream.                           */
) {
    ASSERT_TRAP(point_array != NULL);

    point_array->vertices = NULL;
}

static                          /* Read a list of vertices from the input */
    State_t                     /* stream into the top of the stack,      */
ReadPointArray(                 /* converting the coordinate triples into */
    BinInStream_tp stream,      /* br_vector3 structures.                 */
    Stack_t        *top,
    Parse3dsBinOptions_t *options
) {
    Int_t      i;
    br_uint_16 n_vertices;
    br_vector3 *vertices;
    Float_t    coord;

    ASSERT_TRAP(stream != NULL && top != NULL);

    if (!ReadUInt16(stream,&n_vertices)
          || n_vertices == 0) {
        return PARSE_ERROR;
    }
    top->done += sizeof(br_uint_16);

    vertices = (br_vector3*)BrMemAllocate(n_vertices * sizeof(br_vector3),
                                          BR_MEMORY_APPLICATION);

    if (vertices == NULL) {
        return OUT_OF_MEMORY;
    }
    for (i=0; i < n_vertices; i++) {
        if (!ReadPoint(stream,top,vertices+i,options)) {
            BrMemFree(vertices);
            return PARSE_ERROR;
        }
    }
    top->data.point_array.n_vertices = n_vertices;
    top->data.point_array.vertices   = vertices;

    return OK;
}

static                         /* Deallocate the memory used by the array */
    void                       /* of vertices.                            */
DismantlePointArray(
    PointArray_t *point_array
) {
    ASSERT_TRAP(point_array != NULL);

    if (point_array->vertices != NULL) {
        BrMemFree(point_array->vertices);
        point_array->vertices = NULL;
    }
}


static                         /* Initialise a new face array to not yet  */
    void                       /* contain any information about the faces */
InitialiseFaceArray(           /* shortly to be read in from the input    */
    FaceArray_t *face_array    /* stream.                                 */
) {
    ASSERT_TRAP(face_array != NULL);

    face_array->faces          = NULL;
    face_array->smooth_group   = NULL;
    face_array->msh_mat_groups = NULL;
}


static
    State_t
ReadFaceArray(                 /* Read a list of faces from the input stream */
    BinInStream_tp stream,     /* into the top of the stack, converting      */
    Stack_t        *top        /* the edge visibility flags into BRender     */
) {                            /* coplanarity flags.                         */
    Int_t      i,j;
    br_uint_16 n_faces;
    Face_t     *faces;
    br_uint_16 visibility;

    ASSERT_TRAP(stream != NULL && top != NULL);

    if (!ReadUInt16(stream,&n_faces)
          || n_faces == 0) {
        return PARSE_ERROR;
    }
    top->done += sizeof(br_uint_16);

    faces = (Face_t*)BrMemAllocate(n_faces * sizeof(Face_t),
                                   BR_MEMORY_APPLICATION);

    if (faces == NULL) {
        return OUT_OF_MEMORY;
    }
    for (i=0; i<n_faces; i++) {
        for (j=0; j<3; j++) {
            if (ReadUInt16(stream,&(faces[i].vertices[j]))) {
                top->done += sizeof(br_uint_16);
            } else {
                BrMemFree(faces);
                return PARSE_ERROR;
            }
        }
        if (ReadUInt16(stream,&visibility)) {
            faces[i].flags = 0;
            if ((visibility & 0x0001) == 0) {
                faces[i].flags |= BR_FACEF_COPLANAR_0;
            }
            if ((visibility & 0x0002) == 0) {
                faces[i].flags |= BR_FACEF_COPLANAR_1;
            }
            if ((visibility & 0x0004) == 0) {
                faces[i].flags |= BR_FACEF_COPLANAR_2;
            }
#if 1 /* SAML */
			/*
			 * Transfer the little known wrap flags to brender face flags
			 */
            if (visibility & 0x0008)
                faces[i].flags |= FACEF_UWRAP;

            if (visibility & 0x0010)
                faces[i].flags |= FACEF_VWRAP;
#endif
            top->done += sizeof(br_uint_16);
        } else {
            BrMemFree(faces);
            return PARSE_ERROR;
        }
    }
    top->data.face_array.n_faces = n_faces;
    top->data.face_array.faces = faces;

    return OK;
}


static                        /* Deallocate any memory used by the array of */
    void                      /* faces, their smoothing groups, and their   */
DismantleFaceArray(           /* material references.                       */
    FaceArray_t *face_array
) {
    ASSERT_TRAP(face_array != NULL);

    if (face_array->faces != NULL) {
        BrMemFree(face_array->faces);
        face_array->faces = NULL;
    }
    if (face_array->smooth_group != NULL) {
        BrMemFree(face_array->smooth_group);
        face_array->smooth_group = NULL;
    }
    DeallocateMshMatGroups(face_array->msh_mat_groups);
    face_array->msh_mat_groups = NULL;
}


static                       /* Read a list of material references for a set */
    State_t                  /* of faces from the input stream, into the top */
ReadMshMatGroup(             /* of the stack, remembering to make sure the   */
    BinInStream_tp stream,   /* material for those faces has been defined.   */
    Stack_t        *top,
    MaterialList_t *material_list
) {
    Int_t          i;
    char           buffer[17];
    MaterialList_t *material_link;
    MshMatGroup_t  *msh_mat_group;
    br_uint_16     n_indexes;
    br_uint_16     *indexes;

    ASSERT_TRAP(stream != NULL && top != NULL);

    if (!ReadString(stream,top,17,buffer)
          || !ReadUInt16(stream,&n_indexes)) {
        return PARSE_ERROR;
    }
    top->done += sizeof(br_uint_16);

    material_link = LookupMaterial(buffer,material_list);

    if (material_link == NULL) {
        return PARSE_ERROR;
    }

    msh_mat_group = (MshMatGroup_t*)BrMemAllocate(sizeof(MshMatGroup_t),
                                                  BR_MEMORY_APPLICATION);
    if (msh_mat_group == NULL) {
        return OUT_OF_MEMORY;
    }

	if(n_indexes == 0) {
	    msh_mat_group->indexes = NULL;
	} else {

	    indexes = (br_uint_16*)BrMemAllocate(n_indexes * sizeof(br_uint_16),
                                         BR_MEMORY_APPLICATION);

    	if (indexes == NULL) {
	        BrMemFree(msh_mat_group);
	        return OUT_OF_MEMORY;
	    }
    	for (i=0; i<n_indexes; i++) {
	        if (ReadUInt16(stream,&(indexes[i]))) {
	            top->done += sizeof(br_uint_16);
	        } else {
	            BrMemFree(indexes);
	            BrMemFree(msh_mat_group);
	            return OUT_OF_MEMORY;
	        }
	    }
	    msh_mat_group->indexes = indexes;
	}

    msh_mat_group->material_link  = material_link;
    msh_mat_group->n_indexes      = n_indexes;

    top->data.msh_mat_group = msh_mat_group;

    return OK;
}


static                            /* Deallocate the memory used to assign  */
    void                          /* a group of faces a specific material. */
DeallocateMshMatGroups(
    MshMatGroup_t *msh_mat_groups
) {
    MshMatGroup_t *dud;

    while (msh_mat_groups != NULL) {
         dud = msh_mat_groups;
         msh_mat_groups = msh_mat_groups->next;

         if(dud->indexes)
			BrMemFree(dud->indexes);

         BrMemFree(dud);
    }
}


static                            /* Read a smoothing group definition for   */
    State_t                       /* each face  from the input stream, and   */
ReadSmoothGroup(                  /* convert them from 32 bit to to 16 bit   */
    BinInStream_tp       stream,  /* smoothing groups before placing tham at */
    Stack_t              *top,    /* the top of the stack.                   */
    br_uint_16           n_faces,
    Parse3dsBinOptions_t *options
) {
    br_uint_16 i,j;
    br_uint_16 n_used;
    br_uint_16 set[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    };
    br_uint_16 map[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    br_uint_16 *unused, *used, member;
    br_uint_32 *smooth_group, face_group;
    br_uint_16 *new_smooth_group, new_face_group;

    ASSERT_TRAP(stream != NULL && top != NULL
                   && n_faces > 0 && options != NULL);

    smooth_group = (br_uint_32*)BrMemAllocate(n_faces * sizeof(br_uint_32),
                                              BR_MEMORY_APPLICATION);

    if (smooth_group == NULL) {
        return OUT_OF_MEMORY;
    }
    new_smooth_group = (br_uint_16*)BrMemAllocate(n_faces * sizeof(br_uint_16),
                                                  BR_MEMORY_APPLICATION);

    if (new_smooth_group == NULL) {
        BrMemFree(smooth_group);
        return OUT_OF_MEMORY;
    }
    for (i=0; i<n_faces; i++) {
        if (ReadUInt32(stream,&(smooth_group[i]))) {
            top->done += sizeof(br_uint_32);
        } else {
            BrMemFree(new_smooth_group);
            BrMemFree(smooth_group);
            return OUT_OF_MEMORY;
        }
    }
    i       = 0;
    n_used  = 0;
    used    = set + 32;

    do {
        face_group = smooth_group[i++];
        unused     = set;
        do {
            member = *unused;
            if ((face_group & (1 << member)) != 0) {
                map[member] = (br_uint_16)((1 << n_used) & 0xFFFF);
                n_used += 1;
                used -= 1;
                *unused = *used;
                *used = member;
            } else {
                unused += 1;
            }
        } while (unused < used);
    } while (i < n_faces && n_used < 32);

    if (n_used > 16) {
        if (options->log != NULL) {
            fprintf(options->log,"  %d smoothing groups "
                                 "(arbitrary remapping)\n",n_used);
        }
        for (i=0; i<n_faces; i++) {
            face_group = smooth_group[i];
            new_smooth_group[i] = (br_uint_16)(face_group & 0xFFFF)
                                | (br_uint_16)(face_group >> 16);
        }
    } else {
        if (options->log != NULL) {
            fprintf(options->log,"  %d smoothing groups "
                                 "(sensible remapping)\n",n_used);
        }
        for (i=0; i<n_faces; i++) {
            face_group     = smooth_group[i];
            new_face_group = 0;
            for (j=0; j<n_used; j++) {
                member = used[j];
                if ((face_group & (1 << member)) != 0) {
                    new_face_group |= map[member];
                }
            }
            new_smooth_group[i] = new_face_group;
        }
    }
    BrMemFree(smooth_group);

    top->data.smooth_group = new_smooth_group;

    return OK;
}


static                      /* Initialise a list of texture map coordinates */
    void                    /* in readyness for them being read in from the */
InitialiseTexVerts(         /* input stream.                                */
    TexVerts_t *tex_verts
) {
    ASSERT_TRAP(tex_verts != NULL);

    tex_verts->texverts = NULL;
}


static                      /* Read a list of texture coordinates for every  */
    State_t                 /* vertex from the input stream, converting them */
ReadTexVerts(               /* into br_vector2 structures, before placing    */
    BinInStream_tp stream,  /* them at the top of the stack.                 */
    Stack_t        *top
) {
    Int_t      i;
    br_uint_16 n_texverts;
    br_vector2 *texverts;
    Float_t    u,v;

    ASSERT_TRAP(stream != NULL && top != NULL);

    if (!ReadUInt16(stream,&n_texverts)
          || n_texverts == 0) {
        return PARSE_ERROR;
    }
    top->done += sizeof(br_uint_16);

    texverts = (br_vector2*)BrMemAllocate(n_texverts * sizeof(br_vector2),
                                          BR_MEMORY_APPLICATION);

    if (texverts == NULL) {
        return OUT_OF_MEMORY;
    }
    for (i=0; i<n_texverts; i++) {
        if (ReadFloat(stream,&u) && ReadFloat(stream,&v)) {
            texverts[i].v[0] = BrFloatToScalar((float)(u));
            texverts[i].v[1] = BrFloatToScalar((float)(v));
            top->done += 2 * sizeof(Float_t);
        } else {
            BrMemFree(texverts);
            return PARSE_ERROR;
        }
    }
    top->data.tex_verts.n_texverts = n_texverts;
    top->data.tex_verts.texverts   = texverts;

    return OK;
}


static                        /* Deallocate the memory used to store a   */
    void                      /* texture map coordinate for each vertex. */
DismantleTexVerts(
    TexVerts_t *tex_verts
) {
    ASSERT_TRAP(tex_verts != NULL);

    if (tex_verts->texverts != NULL) {
        BrMemFree(tex_verts->texverts);
        tex_verts->texverts = NULL;
    }
}


static                       /* Read a mesh matrix from the input stream */
    State_t                  /* into the top of the stack, converting it */
ReadMeshMatrix(              /* into a right-handed co-ordinate system.  */
    BinInStream_tp stream,
    Stack_t        *top,
    Parse3dsBinOptions_t *options
) {
    Int_t          i,j;
    br_matrix34    *mesh_matrix;
//    Float_t        element[4][3];
  	br_vector3		row;

    ASSERT_TRAP(stream != NULL && top != NULL);

    mesh_matrix = &(top->data.mesh_matrix);

    for (i=0; i<4; i++) {

		if(!ReadPoint(stream, top, &row, options))
			return PARSE_ERROR;

		mesh_matrix->m[i][0] = row.v[0];
		mesh_matrix->m[i][1] = row.v[1];
		mesh_matrix->m[i][2] = row.v[2];
    }

    return OK;
}


static               /* Allocate a new named object structure, setting its */
    NamedObj_t *     /* name to the one supplied (if non-null).            */
AllocateNamedObj(
    char *name
) {
    NamedObj_t *obj;

    obj = (NamedObj_t*)BrMemAllocate(sizeof(NamedObj_t),BR_MEMORY_APPLICATION);

    if (obj != NULL) {
        if (name != NULL) {
            obj->name = BrMemStrDup(name);
            if (obj->name == NULL) {
                BrMemFree(obj);
                return NULL;
            }
        }
        obj->type = NONE;

        BrMatrix34Identity(&(obj->to_world));
        BrMatrix34Identity(&(obj->from_world));

        obj->next = NULL;
    }
    return obj;
}


static                    /* Deallocate an entire list of properly converted */
    void                  /* named objects.                                  */
DeallocateNamedObjs(
    NamedObj_t *objects
) {
    NamedObj_t *dud;

    while (objects != NULL) {
        dud = objects;
        objects = objects->next;

        BrMemFree(dud->name);

        if (dud->type == LIGHT && dud->data.light != NULL) {
            BrMemFree(dud->data.light);
	} else if (dud->type == CAMERA && dud->data.camera != NULL) {
	    BrMemFree(dud->data.camera);
        } else if (dud->type == MODEL) {
            if (dud->data.model->fill_model != NULL) {
                BrModelFree(dud->data.model->fill_model);
            }
            if (dud->data.model->wire_model != NULL) {
                BrModelFree(dud->data.model->wire_model);
            }
            BrMemFree(dud->data.model);
        }
        BrMemFree(dud);
    }
}


static             /* Take a face map of materials, and a vertex remapping */
    br_model *     /* array, allocate a new br_model, and fill it up.      */
MakeModelFromMaps(
    NTriObj_t      *n_tri_obj,
    Bool_t         (*material_type)(MaterialList_t*),
    char           *name,
    char           *suffix,
    Int_t          n_vertices,
    Int_t          n_faces,
    br_uint_16     *vertex_map,
    MaterialList_t **face_map
) {
    Int_t     i,j;

    br_model  *result;

    br_model  *model;
    br_vertex *vertex;
    br_face   *face;
    char      *full_name;

    ASSERT_TRAP(n_tri_obj != NULL && material_type != NULL
                  && name != NULL && suffix != NULL
                  && vertex_map != NULL && face_map != NULL
                  && (((n_vertices == 0) && (n_faces == 0))
                      || ((n_vertices > 0) && (n_faces > 0))));

    if (n_vertices == 0) {
        return NULL;
    }

    full_name = BrMemAllocate((strlen(name)+strlen(suffix)+1) * sizeof(char),
                              BR_MEMORY_APPLICATION);

    if (full_name == NULL)
        return NULL;

    strcpy(full_name,name);
    strcat(full_name,suffix);

    model = BrModelAllocate(full_name,n_vertices,n_faces);

    BrMemFree(full_name);

    if (model == NULL) {
        return NULL;
    }

    BrVector3SetFloat(&(model->pivot),0.0,0.0,0.0);

    /* For all of the points in the point array.... */

    for (i=0; i<n_tri_obj->point_array.n_vertices; i++) {

        /* If a point has been remapped into the target vertex array.... */

        if (vertex_map[i] != FENCE) {

            /* Find the target vertex */

            vertex = model->vertices + vertex_map[i];

            /* Copy its coordinates from the the current point */

            BrVector3Copy(&(vertex->p),
                          &(n_tri_obj->point_array.vertices[i]));

            /* And if texture mapping is specified.... */

            if (n_tri_obj->tex_verts.texverts != NULL) {

                /* Copy its texture coordinates from the point */

                BrVector2Copy(&(vertex->map),
                              &(n_tri_obj->tex_verts.texverts[i]));
            }
        }
    }

    /* Start the target face at the beginning of the target array */

    face = model->faces;

    /* For all source faces in the mesh.... */

    for (i=0; i<n_tri_obj->face_array.n_faces; i++) {

        /* If a source face has the right type of material shading.... */

        if (material_type(face_map[i])) {

            /* Copy its vertex indices from the vertex remapping array */

            for (j=0; j<3; j++) {
                face->vertices[j]
                     = vertex_map[n_tri_obj->face_array.faces[i].vertices[j]];
            }

            /* Copy the co-planarity flags */

            face->flags = n_tri_obj->face_array.faces[i].flags;

            /* Find the faces material reference */

            if (face_map[i] != NULL) {
                face->material = face_map[i]->material;
            } else {
                face->material = NULL;
            }

            /* If smoothing is specified, then copy the smooth group */

            if (n_tri_obj->face_array.smooth_group != NULL) {
                face->smoothing = n_tri_obj->face_array.smooth_group[i];
            } else {
                face->smoothing = 0;
            }
            face += 1;
        }
    }

    result = BrModelAdd(model);

    if (result == NULL) {
        BrModelFree(model);
        return NULL;
    }

    return result;

}

static
    void
ApplyMatrixToPointArray(       /* Apply "matrix" to all of the vertices in */
    PointArray_t *point_array, /* the point array, to get the co-ordinates */
    br_matrix34  *matrix       /* in local object space.                   */
) {
    Int_t       i;
    br_vector3  new_vertex;

    ASSERT_TRAP(point_array != NULL && matrix != NULL);

    for (i = 0; i < point_array->n_vertices; i++) {

        /* We don't know if the BrMatrix#Apply# method works if the input */
        /* and output vectors are the same, so we use a temporary */

        BrMatrix34ApplyP(&new_vertex, &(point_array->vertices[i]), matrix);
        BrVector3Copy(&(point_array->vertices[i]), &new_vertex);
    }
}


static                         /* Find the inverse of the "to_world" matrix */
    void                       /* and set "from_world" to it, unless it is  */
CreateFromWorldMatrix(         /* singular, when set both to the identity   */
    NamedObj_t *named_obj,     /* matrix.                                   */
    Parse3dsBinOptions_t *options
) {
    br_scalar det;
    br_matrix34 product;
    Int_t i, j;

    det = BrMatrix34Inverse(&(named_obj->from_world),
                            &(named_obj->to_world));
    if (BrScalarToFloat(det) == 0.0) {
        fprintf(options->log,
                "  Singular matrix found, using identity instead.\n");
        BrMatrix34Identity(&(named_obj->from_world));
        BrMatrix34Identity(&(named_obj->to_world));
    }
}

static                /* Makes a Model_t and puts it into the NamedObj_t */
    State_t
ConvertNTriObj(       
    NTriObj_t  *n_tri_obj,
    NamedObj_t *named_obj,
    Parse3dsBinOptions_t *options
) {
    char           *name;
    Model_t        *model;
    Int_t          i,j,k;

    Int_t          n_vertices, n_faces;
    Int_t          n_fill_faces, n_fill_vertices;
    Int_t          n_wire_faces, n_wire_vertices;

    char           *suffix;

    MshMatGroup_t  *group, *msh_mat_groups;
    Face_t         *faces;
    MaterialList_t **face_map;
    br_uint_16     *fill_vertex_map;
    br_uint_16     *wire_vertex_map;

    br_scalar det;

    ASSERT_TRAP(n_tri_obj != NULL && named_obj != NULL);

    n_vertices     = n_tri_obj->point_array.n_vertices;
    n_faces        = n_tri_obj->face_array.n_faces;
    faces          = n_tri_obj->face_array.faces;
    msh_mat_groups = n_tri_obj->face_array.msh_mat_groups;
    name           = named_obj->name;

    named_obj->type = MODEL;

	if(options->apply_meshmat) {
	    BrMatrix34Copy(&(named_obj->to_world), &(n_tri_obj->mesh_matrix));
	    CreateFromWorldMatrix(named_obj, options);
	} else {
        BrMatrix34Identity(&(named_obj->from_world));
        BrMatrix34Identity(&(named_obj->to_world));
	}

    /* Convert points to local coordinate space. */

    ApplyMatrixToPointArray(&(n_tri_obj->point_array),
                            &(named_obj->from_world));

    /* If texture mapping is specified, make sure that the number of */
    /* texture coordinates agrees with the number of vertices.       */

    if (n_tri_obj->tex_verts.texverts != NULL) {
        if (n_tri_obj->tex_verts.n_texverts != n_vertices) {
            return PARSE_ERROR;
        }
    }
    model = BrMemAllocate(sizeof(Model_t),BR_MEMORY_APPLICATION);

    if (model == NULL) {
        return OUT_OF_MEMORY;
    }
    face_map = (MaterialList_t**)BrMemAllocate(
                                      n_faces * sizeof(MaterialList_t*),
                                      BR_MEMORY_APPLICATION);
    if (face_map == NULL) {
        BrMemFree(model);
        return OUT_OF_MEMORY;
    }
    fill_vertex_map = (br_uint_16*)BrMemAllocate(
                                      n_vertices * sizeof(br_uint_16),
                                      BR_MEMORY_APPLICATION);
    if (fill_vertex_map == NULL) {
        BrMemFree(face_map);
        BrMemFree(model);
        return OUT_OF_MEMORY;
    }
    wire_vertex_map = (br_uint_16*)BrMemAllocate(
                                      n_vertices * sizeof(br_uint_16),
                                      BR_MEMORY_APPLICATION);
    if (wire_vertex_map == NULL) {
        BrMemFree(fill_vertex_map);
        BrMemFree(face_map);
        BrMemFree(model);
        return OUT_OF_MEMORY;
    }


    /* Initialise the entire face mapping array to the default material */

    for (i=0; i<n_faces; i++) {
        face_map[i] = NULL;
    }

    /* For all of the different material applied to faces.... */

    for (group = msh_mat_groups; group != NULL; group = group->next) {

         /* For all of the faces given this material.... */

         for (i=0; i<group->n_indexes; i++) {

             /* Find the specific face, and set its entry in the face */
             /* mapping array to the material.                        */

             j = group->indexes[i];
             if (j >= n_faces) {
                 BrMemFree(wire_vertex_map);
                 BrMemFree(fill_vertex_map);
                 BrMemFree(face_map);
                 BrMemFree(model);
                 return PARSE_ERROR;
             } else if (face_map[j] == NULL) {
                 face_map[j] = group->material_link;
             }
         }
    }

    /* Initialise both vertex remapping arrays to remap no vertices. */

    for (i=0; i<n_vertices; i++) {
        fill_vertex_map[i] = FENCE;
        wire_vertex_map[i] = FENCE;
    }
    n_fill_faces    = 0;
    n_wire_faces    = 0;
    n_fill_vertices = 0;
    n_wire_vertices = 0;

    /* For all faces in the face mapping array.... */

    for (i=0; i<n_faces; i++) {

        if (FillMat(face_map[i])) {  /* If it uses a filled material.... */
            n_fill_faces += 1;

            /* For all three vertices referred to by the face.... */

            for (j=0; j<3; j++) {

                /* Find the location of the vertex in the point array,  */
                /* and if no entry for it exists in the filled vertex   */
                /* remapping array, set its entry to the current number */
                /* of filled faces.                                     */

                k = faces[i].vertices[j];
                if (k >= n_vertices) {
                    BrMemFree(wire_vertex_map);
                    BrMemFree(fill_vertex_map);
                    BrMemFree(face_map);
                    BrMemFree(model);
                    return PARSE_ERROR;
                } else if (fill_vertex_map[k] == FENCE) {
                    fill_vertex_map[k] = n_fill_vertices++;
                }
            }
        } else {                     /* If it uses a wriframed material.... */
            n_wire_faces += 1;

            /* For all three vertices referred to by the face.... */

            for (j=0; j<3; j++) {

                /* Find the location of the vertex in the point array,    */
                /* and if no entry for it exists in the wireframed vertex */
                /* remapping array, set its entry to the current number   */
                /* of wireframed faces.                                   */

                k = faces[i].vertices[j];
                if (k >= n_vertices) {
                    BrMemFree(wire_vertex_map);
                    BrMemFree(fill_vertex_map);
                    BrMemFree(face_map);
                    BrMemFree(model);
                    return PARSE_ERROR;
                } else if (wire_vertex_map[k] == FENCE) {
                    wire_vertex_map[k] = n_wire_vertices++;
                }
            }
        }
    }

	if(n_fill_faces && n_wire_faces) {
	    model->fill_model = MakeModelFromMaps(n_tri_obj, &FillMat,
    	                                      name, "-fill",
        	                                  n_fill_vertices, n_fill_faces,
            	                              fill_vertex_map, face_map);
	    model->wire_model = MakeModelFromMaps(n_tri_obj, &WireMat,
    	                                      name, "-wire",
        	                                  n_wire_vertices, n_wire_faces,
            	                              wire_vertex_map, face_map);
	} else if(n_fill_faces) {
	    model->fill_model = MakeModelFromMaps(n_tri_obj, &FillMat,
    	                                      name, "",
        	                                  n_fill_vertices, n_fill_faces,
            	                              fill_vertex_map, face_map);
	} else if(n_wire_faces) {
	    model->wire_model = MakeModelFromMaps(n_tri_obj, &WireMat,
    	                                      name, "",
        	                                  n_wire_vertices, n_wire_faces,
            	                              wire_vertex_map, face_map);
	}

    BrMemFree(wire_vertex_map);
    BrMemFree(fill_vertex_map);
    BrMemFree(face_map);

    if ((n_fill_faces > 0 && model->fill_model == NULL)
          || (n_wire_faces > 0 && model->wire_model == NULL)) {
        if (model->fill_model != NULL) {
            BrModelFree(model->fill_model);
        }
        if (model->wire_model != NULL) {
            BrModelFree(model->wire_model);
        }
        BrMemFree(model);
        return OUT_OF_MEMORY;
    }
    named_obj->data.model = model;

    return OK;
}

static                     /* Generate a transform from a combination of */
    void                   /* the position, target and bank angle of an  */
MakeMatrixFromTarget(      /* object. Used to generate "to_world" for    */
    br_matrix34 *trans,    /* cameras and lights.                        */
    br_vector3 *posn,
    br_vector3 *target,
    Float_t bank_angle_deg
) {
    br_vector3 viewvec, v_normed;
    br_angle bank_angle_br, altitude, azimuth;
    br_fraction x, y, z, out_radius;

    BrVector3Sub(&viewvec, target, posn);
    BrVector3Normalise(&v_normed, &viewvec);

    bank_angle_br = BR_ANGLE_DEG((float) bank_angle_deg);
    x = BrScalarToFraction(v_normed.v[0]);
    y = BrScalarToFraction(v_normed.v[1]);
    z = BrScalarToFraction(v_normed.v[2]);
    out_radius = BrScalarToFraction(BR_LENGTH2(v_normed.v[0], v_normed.v[2]));
    altitude = BR_ATAN2(y, out_radius);
    if (x != BR_SCALAR(0) || z != BR_SCALAR(0)) {
        azimuth = BR_ATAN2(x, z);
    } else {
        azimuth = BR_ANGLE_DEG(90.0);
    }
    BrMatrix34Translate(trans, posn->v[0], posn->v[1], posn->v[2]);
    BrMatrix34PreRotateY(trans, azimuth);
    BrMatrix34PreRotateX(trans, BR_CONST_MUL(altitude, -1));
    BrMatrix34PreRotateZ(trans, bank_angle_br);
    BrMatrix34PreRotateY(trans, BR_ANGLE_DEG(180.0));
}


static                /* Makes a Camera_t and puts it into the NamedObj_t */
    State_t
ConvertNCamera(
    NCamera_t  *n_camera,
    NamedObj_t *named_obj,
    Parse3dsBinOptions_t *options
) {
    Camera_t *camera;
    float fov;

    MakeMatrixFromTarget(&(named_obj->to_world), &(n_camera->posn),
                         &(n_camera->target), n_camera->bank_angle);
    CreateFromWorldMatrix(named_obj, options);

    /* If the no_cameras flag is set, then set the type of object to */
    /* NONE, so that dummy actors will be made for it, otherwise.... */

    if (options->no_cameras) {
        named_obj->type = NONE;
    } else {

        /* Set the type of named object to be a camera, and convert */
        /* the focal length to a field of view angle.               */        

        named_obj->type = CAMERA;
        named_obj->data.camera = camera = 
            (Camera_t *) BrMemAllocate(sizeof(Camera_t),
                                       BR_MEMORY_APPLICATION);
	if (camera == NULL) {
	    return OUT_OF_MEMORY;
	}
        fov = (float) 2 * atan2(HALF_IMAGE_PLANE_MM, n_camera->focal_length);

        if (options->log != NULL) {
            fprintf(options->log, 
                    "Focal length: %5.1f mm Angle: %5.1f degrees\n",
                    n_camera->focal_length,
                    180.0 * fov / 3.141593);
        }
        camera->field_of_view = BrRadianToAngle(BrFloatToScalar(fov));
    }

    return OK;
}

static               /* Makes a Light_t and puts it into the NamedObj_t */
    State_t
ConvertNDLight(
    NDLight_t  *n_d_light,
    NamedObj_t *named_obj,
    Parse3dsBinOptions_t *options
) {
    Light_t *light;

    if (n_d_light->is_spotlight) {
        MakeMatrixFromTarget(&(named_obj->to_world), &(n_d_light->posn),
                             &(n_d_light->dl_spotlight.target), 0.0);
    } else {
        BrMatrix34Translate(&(named_obj->to_world),
                            n_d_light->posn.v[0],
                            n_d_light->posn.v[1],
                            n_d_light->posn.v[2]);
    }
    CreateFromWorldMatrix(named_obj, options);

    /* If the no_cameras flag is set, then set the type of object to */
    /* NONE, so that dummy actors will be made for it, otherwise.... */

    if (options->no_lights) {
        named_obj->type = NONE;
    } else {

        /* Set the type of the named object to be a light. */

        named_obj->type = LIGHT;

        named_obj->data.light = light
            = (Light_t *) BrMemAllocate(sizeof(Light_t),
                                        BR_MEMORY_APPLICATION);
        if (light == NULL) {
            return OUT_OF_MEMORY;
        }

        /* Copy the on-switch position, convert the Color_t to a br_colour */

        light->is_off = n_d_light->is_off;
        light->colour = ConvertColor(&(n_d_light->color));

        if (n_d_light->is_spotlight) {

            /* If its a spotlight, then set the type of light to a be a */
            /* BR_LIGHT_SPOT, and convert the cone angles.              */

            light->type = BR_LIGHT_SPOT;
            light->cone_inner = BrDegreeToAngle(BrFloatToScalar((float)(
                n_d_light->dl_spotlight.cone_inner)));
            light->cone_outer = BrDegreeToAngle(BrFloatToScalar((float)(
                n_d_light->dl_spotlight.cone_inner)));
        } else {

            /* If its not a spotlight, just set the type */

            light->type = BR_LIGHT_POINT;
        }
    }
    return OK;
}


static                    /* Initialise all of the component parts of a */
    void                  /* triangle mesh object, ready to read the    */
InitialiseNTriObj(        /* enclosed chunks.                           */
    NTriObj_t *n_tri_obj
) {
    Int_t i,j;

    ASSERT_TRAP(n_tri_obj != NULL);

    InitialisePointArray(&(n_tri_obj->point_array));
    InitialiseFaceArray(&(n_tri_obj->face_array));
    InitialiseTexVerts(&(n_tri_obj->tex_verts));
}


static                   /* Deallocate all the memory used to hold a */
    void                 /* 3DStudio triangle mesh.                  */
DismantleNTriObj(        
    NTriObj_t *n_tri_obj
) {
    ASSERT_TRAP(n_tri_obj != NULL);

    DismantlePointArray(&(n_tri_obj->point_array));
    DismantleFaceArray(&(n_tri_obj->face_array));
    DismantleTexVerts(&(n_tri_obj->tex_verts));
}


static                  /* Make an array of pointers to br_models, for the */
    br_model **         /* BrModelSaveMany() call.                         */
CollectModels(
    NamedObj_t *list,
    Int_t      *number
) {
    Int_t         i;
    br_model      **result;
    NamedObj_t    *item;

    ASSERT_TRAP(number != NULL);

    *number = 0;

    for (item = list; item != NULL; item = item->next) {
        if (item->type == MODEL) {
            if (item->data.model->fill_model != NULL) {
                *number += 1;
            }
            if (item->data.model->wire_model != NULL) {
                *number += 1;
            }
        }
    }
    if ((*number) == 0) {
        return NULL;
    }
    result = (br_model**)BrMemAllocate((*number) * sizeof(br_model*),
                                       BR_MEMORY_APPLICATION);

    if (result != NULL) {
        i = 0;
        for (item = list; item != NULL; item = item->next) {
            if (item->type == MODEL) {
                if (item->data.model->fill_model != NULL) {
                    result[i++] = item->data.model->fill_model;
                }
                if (item->data.model->wire_model != NULL) {
                    result[i++] = item->data.model->wire_model;
                }
            }
        }
    }
    return result;
}

static                /* Make an array of pointers to br_models, for the */
    br_material **    /* BrMateriallSaveMany() call.                     */
CollectMaterials(
    MaterialList_t *list,
    Int_t          *number
) {
    Int_t          i;
    br_material    **result;
    MaterialList_t *item;

    ASSERT_TRAP(number != NULL);

    *number = 0;

    for (item = list; item != NULL; item = item->next) {
        *number += 1;
    }
    if ((*number) == 0) {
        return NULL;
    }
    result = (br_material**)BrMemAllocate((*number) * sizeof(br_material*),
                                          BR_MEMORY_APPLICATION);

    if (result != NULL) {
        i = 0;
        for (item = list; item != NULL; item = item->next) {
            result[i++] = item->material;
        }
    }
    return result;
}


static                             /* Read a light from the input stream and */
    State_t                        /* leave it pushed onto the stack.        */
ReadNDLight(
    BinInStream_tp       stream,
    Stack_t              *top,
    Parse3dsBinOptions_t *options
) {
    NDLight_t *n_d_light;

    ASSERT_TRAP(stream != NULL && top != NULL && options != NULL);

    n_d_light = &(top->data.n_d_light);

    n_d_light->is_off       = FALSE;
    n_d_light->is_spotlight = FALSE;

    if (!ReadPoint(stream,top,&(n_d_light->posn),options)) {
        return PARSE_ERROR;
    }

    if (options->log != NULL) {
        fprintf(options->log,"Light: \"%s\"\n",top[-1].data.named_obj->name);
        fprintf(options->log,"  position: (%f  %f  %f)\n",
                BrScalarToFloat(n_d_light->posn.v[0]),
                BrScalarToFloat(n_d_light->posn.v[1]),
                BrScalarToFloat(n_d_light->posn.v[2]));
    }

    return OK;
}

static                   /* Read a spotlight from the input stream and leave */
    State_t              /* pushed onto the top of the stack.                */
ReadDlSpotlight(
    BinInStream_tp       stream,
    Stack_t              *top,
    Parse3dsBinOptions_t *options
) {

    DlSpotlight_t *dl_spotlight;

    ASSERT_TRAP(stream != NULL && top != NULL && options != NULL);

    dl_spotlight = &(top->data.dl_spotlight);

    if (!ReadPoint(stream,top,&(dl_spotlight->target),options)
          || !ReadFloat(stream,&(dl_spotlight->cone_inner))
          || !ReadFloat(stream,&(dl_spotlight->cone_outer))
          || dl_spotlight->cone_inner < 1.0
          || dl_spotlight->cone_inner > 160.0
          || dl_spotlight->cone_outer < 1.0
          || dl_spotlight->cone_outer > 160.0) {
        return PARSE_ERROR;
    }
    top->done += 2 * sizeof(Float_t);

    if (!SkipRest(stream,top)) {
        return PARSE_ERROR;
    }

    if (options->log != NULL) {
        fprintf(options->log,"  spotlight target: (%f  %f  %f)\n",
                BrScalarToFloat(dl_spotlight->target.v[0]),
                BrScalarToFloat(dl_spotlight->target.v[1]),
                BrScalarToFloat(dl_spotlight->target.v[2]));
        fprintf(options->log,"  cone angles: %f degrees -> %f degrees\n",
                dl_spotlight->cone_inner,
                dl_spotlight->cone_outer);
    }

    return OK;
}


static                   /* Read a camera from the input stream, skipping   */
    State_t              /* all of the enclosed chunks, and leave it pushed */
ReadNCamera(             /* onto the top of the stack.                      */
    BinInStream_tp       stream,
    Stack_t              *top,
    Parse3dsBinOptions_t *options
) {
    NCamera_t *n_camera;

    ASSERT_TRAP(stream != NULL && top != NULL && options != NULL);

    n_camera = &(top->data.n_camera);

    if (!ReadPoint(stream,top,&(n_camera->posn), options)
          || !ReadPoint(stream,top,&(n_camera->target), options)
          || !ReadFloat(stream,&(n_camera->bank_angle))
          || !ReadFloat(stream,&(n_camera->focal_length))) {
        return PARSE_ERROR;
    }

    top->done += 2 * sizeof(Float_t);

    if (!SkipRest(stream,top)) {
        return PARSE_ERROR;
    }

    if (options->log != NULL) {
       fprintf(options->log,"Camera: \"%s\"\n",top[-1].data.named_obj->name);
       fprintf(options->log,"  position: (%f  %f  %f)\n",
               BrScalarToFloat(n_camera->posn.v[0]),
               BrScalarToFloat(n_camera->posn.v[1]),
               BrScalarToFloat(n_camera->posn.v[2]));
       fprintf(options->log,"  target: (%f  %f  %f)\n",
               BrScalarToFloat(n_camera->target.v[0]),
               BrScalarToFloat(n_camera->target.v[1]),
               BrScalarToFloat(n_camera->target.v[2]));
    }

    return OK;
}


static                        /* guess.... */    
    NamedObj_t *
LookupNamedObj(
    char       *name,
    NamedObj_t *named_objs
) {
    NamedObj_t *obj;

    ASSERT_TRAP(name != NULL);

    for (obj = named_objs; obj != NULL; obj = obj->next) {
        if (strcmp(name,obj->name) == 0) {
            return obj;
        }
   }
   return NULL;
}


static                     /* Initialise a node header so that it cannot be  */
    void                   /* referenced as a parent, and has its own parent */
InitialiseNodeHdr(         /* set to the root of the hierarchy.              */
    NodeHdr_t *node_hdr
) {
    ASSERT_TRAP(node_hdr != NULL);

    node_hdr->index     = -2;
    node_hdr->parent    = -1;
    node_hdr->named_obj = NULL;
}


static                      /* Read in a node header from the input stream, */
    State_t                 /* and leave it pushed onto the top of the      */
ReadNodeHdr(                /* stack.                                       */
    BinInStream_tp stream,
    Stack_t        *top,
    Int_t          index,
    NamedObj_t     *named_objs
) {
    char       buffer[12];
    br_uint_16 skip;
    br_int_16  parent;
    NodeHdr_t  *node_hdr;

    ASSERT_TRAP(stream != NULL && top != NULL && index >= 0);

    if (!ReadString(stream,top,12,buffer)
          || !ReadUInt16(stream,&skip)
          || !ReadUInt16(stream,&skip)
          || !ReadInt16(stream,&parent)
          || parent < -1 || parent >= index) {
        return PARSE_ERROR;
    }
    top->done += (sizeof(br_uint_16) * 2) + sizeof(br_int_16);

    node_hdr = &(top->data.node_hdr);

    /* If this is not a dummy node, then try to find the object named */
    /* in the preamble. Fail if it cannot be found.                   */

    if (strcmp(buffer,"$$$DUMMY") != 0) {
        node_hdr->named_obj = LookupNamedObj(buffer,named_objs);
        if (node_hdr->named_obj == NULL) {
            return PARSE_ERROR;
        }
    }

    node_hdr->index  = index;
    node_hdr->parent = parent;

#if SHOW_KEYFRAME_DATA
    DisplayTop("Reading",top-1,stack);
    fprintf(stdout,"  NODE_HDR:  %d  \"%s\"  %d\n",index,buffer,parent);
#endif

    return OK;
}

static               /* Allocate a new node tag, and initialise its node */
    NodeTag_t *      /* header.                                          */
AllocateNodeTag(
) {
    NodeTag_t *node_tag;

    node_tag = (NodeTag_t*)BrMemAllocate(sizeof(NodeTag_t),
                                         BR_MEMORY_APPLICATION);
    if (node_tag != NULL) {
        InitialiseNodeHdr(&(node_tag->node_hdr));
        node_tag->instance_name = NULL;
        node_tag->has_pivot     = FALSE;
        node_tag->has_boundbox  = FALSE;
        node_tag->next          = NULL;
    }
    return node_tag;
}


static
    Bool_t
SameString(
    char *a,
    char *b
) {
    return ((a == NULL && b == NULL)
              || (a != NULL && b != NULL && strcmp(a,b) == 0));
}


static                   /* Insert a new node tag at the end of an existing */
    Bool_t               /* list, making sure that it cannot be confused    */
InsertNodeTag(           /* with any node tags already in the list.         */
    NodeTag_t *new_tag,
    NodeTag_t **tags
) {
    NodeTag_t  *tag;
    NodeTag_t  **link;
    NamedObj_t *named_obj;

    ASSERT_TRAP(new_tag != NULL && tags != NULL);

    named_obj = new_tag->node_hdr.named_obj;

    link = tags;

    if (named_obj == NULL) {
        for (tag = *tags; tag != NULL; tag = tag->next) {
            if (tag->node_hdr.named_obj == NULL
                  && SameString(tag->instance_name,new_tag->instance_name)) {
                return FALSE;
            }
            link = &(tag->next);
        }
    } else {
        for (tag = *tags; tag != NULL; tag = tag->next) {
            if (tag->node_hdr.named_obj != NULL
                  && SameString(named_obj->name,tag->node_hdr.named_obj->name)
                  && SameString(new_tag->instance_name,tag->instance_name)) {
                return FALSE;
            }
            link = &(tag->next);
        }
    }
    *link = new_tag;

    return TRUE;
}

static                    /* Deallocate all of the memory used by an entire */
    void                  /* list of node tags.                             */
DeallocateNodeTags(
    NodeTag_t *node_tags
) {
    NodeTag_t *dud;

    while (node_tags != NULL) {
        dud = node_tags;
        node_tags = node_tags->next;

        if (dud->instance_name != NULL) {
            BrMemFree(dud->instance_name);
        }
        BrMemFree(dud);
   }
}

static                    /* Allocate a new actor with a specific name and */
    br_actor *            /* type. Set it up to have the default rendering */
AllocateNamedActor(       /* style, and have identity as its transform.    */
    char      *name,
    br_uint_8 type
) {
    br_actor *actor;

    actor = BrActorAllocate(type,NULL);

    if (actor == NULL) {
        return NULL;
    }
    actor->render_style = BR_RSTYLE_DEFAULT;
    actor->t.type = BR_TRANSFORM_IDENTITY;

    if (name != NULL) {
        actor->identifier = BrMemStrDup(name);
        if (actor->identifier == NULL) {
            BrActorFree(actor);
            return NULL;
        }
    }
    return actor;
}

static
    void
DisplayActor(
    FILE     *log,
    Int_t    depth,
    br_actor *actor
) {
    Int_t i;
    Int_t indent_depth;

    ASSERT_TRAP(log != NULL && depth >= 0);

    if (actor != NULL) {
        fprintf(log,"%4d  ",depth);

        indent_depth = depth % MAX_DISPLAY_DEPTH;

        if (indent_depth != depth) {
            fprintf(log,"... ");
        }
        for(i=0; i < indent_depth; i++) {
            fprintf(log,"|   ");
        }
        switch (actor->type) {
            case BR_ACTOR_NONE :
                fprintf(log,"BR_ACTOR_NONE");
                break;
            case BR_ACTOR_LIGHT :
                fprintf(log,"BR_ACTOR_LIGHT");
                break;
            case BR_ACTOR_CAMERA :
                fprintf(log,"BR_ACTOR_CAMERA");
                break;
            case BR_ACTOR_MODEL :
                fprintf(log,"BR_ACTOR_MODEL");
                break;
        }
        if (actor->identifier != NULL){
            fprintf(log,": \"%s\"",actor->identifier);
        }
        fprintf(log,"\n");
    }
}

static void TranslateModelVertices(br_model *m, br_scalar x,br_scalar y, br_scalar z)
{
 	int i;

	for(i=0; i < m->nvertices; i++) {
		m->vertices[i].p.v[0] =BR_ADD(m->vertices[i].p.v[0], x);
		m->vertices[i].p.v[1] =BR_ADD(m->vertices[i].p.v[1], y);
		m->vertices[i].p.v[2] =BR_ADD(m->vertices[i].p.v[2], z);
	}
}


static                 /* Create a small hierarchy for the named object */
    br_actor *
MakeEnclosingActor(
    Int_t      depth,
    char       *instance_name,
    NamedObj_t *named_obj,
    br_matrix34 *world_to_parent,     /* converts from world to */
                                      /* parent's coordinate space */
    br_matrix34 *world_to_actor_ret,
	br_vector3	*pivot,
    Parse3dsBinOptions_t *options
) {
    Int_t     i;

    br_actor  *actor;
    char      *actor_name;
    Int_t     name_len;

    br_light  *light;
    br_camera *camera;

    Model_t   *model;

    br_actor  *fill_actor;
    br_actor  *wire_actor;

    br_model  *fill_model;
    br_model  *wire_model;

    actor_name = NULL;

	/*
	 * Work out name for this actor
	 */
    if (named_obj == NULL) {
        if (instance_name != NULL) {
            actor_name = BrMemStrDup(instance_name);
            if (actor_name == NULL) {
                return NULL;
            }
        }
    } else if (instance_name != NULL) {
        name_len   = strlen(named_obj->name) + strlen(instance_name) + 2;
        actor_name = (char*)BrMemAllocate(name_len * sizeof(char),
                                          BR_MEMORY_APPLICATION);
        if (actor_name == NULL) {
            return NULL;
        }
        strcpy(actor_name,named_obj->name);
        strcat(actor_name,".");
        strcat(actor_name,instance_name);
    } else {
        actor_name = BrMemStrDup(named_obj->name);
        if (actor_name == NULL) {
            return NULL;
        }
    }

	/*
	 * Allocate the actor structure
	 */
    if (named_obj != NULL && named_obj->type == CAMERA) {
        actor = AllocateNamedActor(NULL,BR_ACTOR_CAMERA);
    } else if (named_obj != NULL && named_obj->type == LIGHT) {
        actor = AllocateNamedActor(NULL,BR_ACTOR_LIGHT);
    } else {
        actor = AllocateNamedActor(NULL,BR_ACTOR_NONE);
    }
    if (actor == NULL) {
        BrMemFree(actor_name);
        return NULL;
    }
    actor->identifier = actor_name;

    fill_actor = NULL;
    wire_actor = NULL;


    if (named_obj != NULL) {

		if(pivot) {
			BrMatrix34PreTranslate(&(named_obj->to_world),
				pivot->v[0],pivot->v[1],pivot->v[2]);
			BrMatrix34PostTranslate(&(named_obj->from_world),
				-pivot->v[0],-pivot->v[1],-pivot->v[2]);
		}

        /* Do this for node tags that reference a named object. */

        actor->t.type = BR_TRANSFORM_MATRIX34;
        BrMatrix34Mul(&(actor->t.t.mat),
                      &(named_obj->to_world), world_to_parent);

        /* Watt-style matrices, remember... */

        BrMatrix34Copy(world_to_actor_ret, &(named_obj->from_world));
    } else {

        /* Do this for dummy node tags. */

        actor->t.type = BR_TRANSFORM_IDENTITY;
        BrMatrix34Copy(world_to_actor_ret, world_to_parent);
    }

    if (named_obj != NULL && named_obj->type == LIGHT) {

        light = (br_light*)(actor->type_data);

        /* Copy the named object's light into the actors light. */

        light->identifier = BrMemStrDup(named_obj->name);

        if (light->identifier == NULL) {
            BrActorFree(actor);
            return NULL;
        }
        light->colour = named_obj->data.light->colour;
        light->type = named_obj->data.light->type;

        if (light->type == BR_LIGHT_SPOT) {
            light->cone_inner = named_obj->data.light->cone_inner;
            light->cone_outer = named_obj->data.light->cone_outer;
        }
        if (named_obj->data.light->is_off) {
            BrLightDisable(actor);
        } else {
            BrLightEnable(actor);
        }
    } else if (named_obj != NULL && named_obj->type == CAMERA) {

        camera = (br_camera*)(actor->type_data);

        /* Copy the named object's camera into the actors camera. */

        camera->identifier = BrMemStrDup(named_obj->name);
        if (camera->identifier == NULL) {
            BrActorFree(actor);
            return NULL;
        }
        camera->type = BR_CAMERA_PERSPECTIVE;
        camera->field_of_view = named_obj->data.camera->field_of_view;
        camera->hither_z = BR_SCALAR(options->hither);
        camera->yon_z = BR_SCALAR(options->yon);
        camera->aspect = BR_SCALAR(1.6);
    } else if (named_obj != NULL &&
			named_obj->type == MODEL &&
        	named_obj->data.model->fill_model != NULL &&
        	named_obj->data.model->wire_model != NULL) {

        model = named_obj->data.model;

		/* Allocate an actor for the filled faces mesh, and add it */
        /* as a child to the actor allocated at the start of this  */
        /* function.                                               */

        fill_actor = AllocateNamedActor(model->fill_model->identifier,
                                        BR_ACTOR_MODEL);
        if (fill_actor == NULL) {
            BrActorFree(actor);
            return NULL;
        }
        fill_actor->model        = model->fill_model;
        fill_actor->render_style = BR_RSTYLE_FACES;
        fill_actor->t.type = BR_TRANSFORM_IDENTITY;

		if(pivot)
			TranslateModelVertices(wire_actor->model,
				-pivot->v[0],-pivot->v[1],-pivot->v[2]);

        if (BrActorAdd(actor,fill_actor) == NULL) {
            BrActorFree(actor);
            BrActorFree(fill_actor);
            return NULL;
        }

        /* Allocate an actor for the wireframed faces mesh, and add it */
        /* as a child to the actor allocated at the start of this      */
        /* function.                                                   */

        wire_actor = AllocateNamedActor(model->wire_model->identifier,
                                        BR_ACTOR_MODEL);
        if (wire_actor == NULL) {
            BrActorFree(actor);
            return NULL;
        }
        wire_actor->model        = model->wire_model;
        wire_actor->render_style = BR_RSTYLE_EDGES;
        wire_actor->t.type = BR_TRANSFORM_IDENTITY;

		if(pivot)
			TranslateModelVertices(wire_actor->model,
				-pivot->v[0],-pivot->v[1],-pivot->v[2]);

        if (BrActorAdd(actor,wire_actor) == NULL) {
            BrActorFree(actor);
            BrActorFree(wire_actor);
            return NULL;
        }
    } else if (named_obj != NULL &&
			named_obj->type == MODEL &&
        	named_obj->data.model->fill_model != NULL &&
        	named_obj->data.model->wire_model == NULL) {

        actor->model = named_obj->data.model->fill_model;
		actor->render_style = BR_RSTYLE_FACES;
		actor->type = BR_ACTOR_MODEL;

		if(pivot)
			TranslateModelVertices(actor->model,
				-pivot->v[0],-pivot->v[1],-pivot->v[2]);

    } else if (named_obj != NULL &&
			named_obj->type == MODEL &&
        	named_obj->data.model->fill_model == NULL &&
        	named_obj->data.model->wire_model != NULL) {

        actor->model = named_obj->data.model->wire_model;
		actor->type = BR_ACTOR_MODEL;
		actor->render_style = BR_RSTYLE_EDGES;

		if(pivot)
			TranslateModelVertices(actor->model,
				-pivot->v[0],-pivot->v[1],-pivot->v[2]);
	}

    if (options->log != NULL) {
        DisplayActor(options->log, depth, actor);
        DisplayActor(options->log, depth+1, fill_actor);
        DisplayActor(options->log, depth+1, wire_actor);
    }

    return actor;
}



static                 /* Find all of the direct descendents of the node   */
    Bool_t             /* tag with the index "parent", allocate actors for */
CollectDescendents(    /* them, and if they can be recursed on, then add   */
    Int_t     depth,   /* them as children to the "actor" parameter.       */
    br_actor  *actor,
    Int_t     parent,
    NodeTag_t *node_tags,
    br_matrix34 *world_to_parent,
    Parse3dsBinOptions_t *options
) {
    br_actor  *child;
    NodeTag_t *node_tag;
    br_matrix34 world_to_child;

    ASSERT_TRAP(actor != NULL);

    for (node_tag = node_tags; node_tag != NULL; node_tag = node_tag->next) {

        /* Find a node tag with the indicated parent. */

        if (node_tag->node_hdr.parent == parent) {

            /* Allocate its actor. */

            child = MakeEnclosingActor(depth+1, node_tag->instance_name,
                                       node_tag->node_hdr.named_obj,
                                       world_to_parent,
                                       &world_to_child,
										options->apply_pivot?&node_tag->pivot:NULL,
										options);
            if (child == NULL) {
                return FALSE;
            }

            /* Collect its own children, and children's children, etc... */

            if (!CollectDescendents(depth+1, child,
                                    node_tag->node_hdr.index,
                                    node_tags, &world_to_child,
									options)
                  || BrActorAdd(actor,child) == NULL) {
                BrActorFree(child);
                return FALSE;
            }
        }
    }
    return TRUE;
}


static                      /* Allocates a root actor, and start the   */
    br_actor *              /* recursive process that build the actual */
BuildComplexHierarchy(      /* hierarchy.                              */
    NodeTag_t  *node_tags,
    Parse3dsBinOptions_t *options
) {
    br_actor *world;
    br_matrix34 world_to_world;

    world = AllocateNamedActor(NULL,BR_ACTOR_NONE);

    if (world != NULL) {
        if (options->log != NULL) {
            DisplayActor(options->log, 0, world);
        }
        BrMatrix34Identity(&world_to_world);
        if (!CollectDescendents(0, world, -1, node_tags,
                                &world_to_world, options)) {
            BrActorFree(world);
            world = NULL;
        }
    }
    return world;
}


static                      /* Allocate a root actor, and add all of the */
    br_actor *              /* named object actors to it as children.    */
BuildFlatHierarchy(
    NamedObj_t *named_objs,
    Parse3dsBinOptions_t *options
) {
    br_actor *world, *child;
    br_matrix34 world_to_world;
    br_matrix34 dummy;
    NamedObj_t *obj;

    world = AllocateNamedActor(NULL,BR_ACTOR_NONE);

    if (world != NULL) {
        if (options->log != NULL) {
            DisplayActor(options->log, 0, world);
        }
        BrMatrix34Identity(&world_to_world);

        for (obj = named_objs; obj != NULL; obj = obj->next) {
            child = MakeEnclosingActor(1, NULL, obj, 
                                       &world_to_world, &dummy, NULL,
                                       options);
            if (child == NULL) {
                BrActorFree(world);
                return NULL;
            }
            if (BrActorAdd(world,child) == NULL) {
                BrActorFree(world);
                BrActorFree(child);
                return NULL;
            }
        }
    }
    return world;
}


    Bool_t
Parse3dsBin(
    Parse3dsBinOptions_t *options
) {
    BinInStream_tp stream;
    Stack_t        stack[16], *top, *parent;
    State_t        state;

    PixmapList_t   *pixmap_list;

    MaterialList_t *material_list;
    Int_t          n_materials;
    br_material    **all_materials = NULL;

    NamedObj_t     *named_objs;
    Int_t          n_models;
    br_model       **all_models;

    br_actor       *world;

    char           *filename;
    Int_t          n_saved;

    NodeTag_t      *node_tags;
    Int_t          n_node_tags;

    if (options == NULL) {
        fprintf(stdout,"No options supplied.\n");
        return FALSE;
    }

    state         = OK;
    top           = stack;
    stream        = AllocateBinInStream();
    material_list = NULL;
    named_objs    = NULL;
    pixmap_list   = NULL;
    node_tags     = NULL;
    n_node_tags   = 0;


	/*
	 * Start reading the input file
	 */
	if (!OpenBinInStream(stream,options->input_filename)) {
		fprintf(stdout,"Cannot open \"%s\" for reading.\n",
			options->input_filename);
		DeallocateBinInStream(stream);
		return FALSE;
	}

	/*
	 * Read the first header onto the top of the stack - don't look
	 * at what it is, since there are severl options, depending on
	 * how the file was written out
	 */
	if (!ReadHeader(stream,top) || top->length < 6) {
		state = PARSE_ERROR;
#if SHOW_BROKEN_CHUNKS
		DisplayTop("Failed READ FIRST HEADER",top,stack);
#endif
	 }

    while (top >= stack && state == OK) {
        parent = top++;                       /* PUSH */

        if (!ReadHeader(stream,top)
              || top->length < 6
              || top->length > (parent->length - parent->done)) {
            state = PARSE_ERROR;
#if SHOW_BROKEN_CHUNKS
            DisplayTop("Failed READ HEADER",top,stack);
#endif

        } else {
#if SHOW_STACK
            DisplayTop("PUSH",top,stack);
#endif

            switch (top->id_tag) {
		
                case M3DMAGIC:
                    break;

                case MLIBMAGIC:
                    break;

                case MDATA:
                    break;

                case COLOR_F :
                    if ( ((parent->id_tag != AMBIENT_LIGHT)
                            && (parent->id_tag != MAT_AMBIENT)
                            && (parent->id_tag != MAT_DIFFUSE)
                            && (parent->id_tag != MAT_SPECULAR)
                            && (parent->id_tag != N_DIRECT_LIGHT) )
                          || (parent->flags & GOT_COLOR_F) != 0
                          || (parent->flags & GOT_COLOR_24) != 0
                          || !ReadColorF(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case COLOR_24 :
                    if ( ((parent->id_tag != AMBIENT_LIGHT)
                            && (parent->id_tag != MAT_AMBIENT)
                            && (parent->id_tag != MAT_DIFFUSE)
                            && (parent->id_tag != MAT_SPECULAR)
                            && (parent->id_tag != N_DIRECT_LIGHT) )
                          || (parent->flags & GOT_COLOR_F) != 0
                          || (parent->flags & GOT_COLOR_24) != 0
                          || !ReadColor24(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case INT_PERCENTAGE :
                    if ( ( (parent->id_tag == MAT_SHININESS)
                            || (parent->id_tag == MAT_TRANSPARENCY)
                            || (parent->id_tag == MAT_TEXMAP)
                            || (parent->id_tag == MAT_REFLMAP) )
                          && (parent->flags & GOT_INT_PERCENTAGE) == 0
                          && (parent->flags & GOT_FLOAT_PERCENTAGE) == 0) {
                        br_int_16 percent;

                        if (ReadInt16(stream,&percent)
                               && percent >= 0
                               && percent <= 100) {
                            /*
                             *  Note: the lower bound needs checking against
                             * -100 for the (unsupported) MAT_XPFALL parent.
                             */

                            top->data.percent = ((Float_t)(percent)) / 100.0;

                            top->done += sizeof(br_int_16);

                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case FLOAT_PERCENTAGE :
                    if ( ( (parent->id_tag == MAT_SHININESS)
                            || (parent->id_tag == MAT_TRANSPARENCY)
                            || (parent->id_tag == MAT_TEXMAP)
                            || (parent->id_tag == MAT_REFLMAP) )
                          && (parent->flags & GOT_INT_PERCENTAGE) == 0
                          && (parent->flags & GOT_FLOAT_PERCENTAGE) == 0
                          && ReadFloat(stream,&(top->data.percent))
                          && top->data.percent >= 0.0
                          && top->data.percent <= 1.0) {
                         /* Note: the lower bound needs checking against    */
                         /* -1.0 for the (unsupported) MAT_XPFALL parent. */

                         top->done += sizeof(Float_t);

                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_MAPNAME :
                    top->data.string = NULL;
                    if ((parent->id_tag == MAT_TEXMAP
                            || parent->id_tag == MAT_REFLMAP)
                          && (parent->flags & GOT_MAT_MAPNAME) == 0) {
                        char buffer[14];

                        if (ReadString(stream,top,14,buffer)) {
                            top->data.string = BrMemStrDup(buffer);
                            if (top->data.string == NULL) {
                                state = OUT_OF_MEMORY;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;

                case M3D_VERSION :
                    if (parent->id_tag == M3DMAGIC
                          && (parent->flags & GOT_M3D_VERSION) == 0) {
                        br_int_32 version;

                        if (ReadInt32(stream,&version)) {
                            top->done += sizeof(br_int_32);
                            if (options->verbose) {
                                if (version == 3) {
                                    fprintf(stdout,"File version %d.0.\n",
                                            version);
                                } else {
                                    fprintf(stdout,"File version %d.0, only "
                                            "able to read version 3.0 chunks"
                                            "\n",version);
                                }
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;

                case MESH_VERSION :
                    if (parent->id_tag == MDATA
                          && (parent->flags & GOT_MESH_VERSION) == 0) {
                        br_int_32 version;

                        if (ReadInt32(stream,&version)) {
                            top->done += sizeof(br_int_32);
                            if (options->verbose) {
                                if ((version == 1) || (version == 2)) {
                                    fprintf(stdout,"Mesh version %d.0.\n",
                                            version);
                                } else {
                                    fprintf(stdout,"Mesh version %d.0, only "
                                            "able to read versions 1.0 and "
                                            "2.0 chunks.\n",version);
                                }
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MASTER_SCALE :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_MASTER_SCALE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case LO_SHADOW_BIAS :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_LO_SHADOW_BIAS) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case HI_SHADOW_BIAS :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_HI_SHADOW_BIAS) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SHADOW_MAP_SIZE :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_SHADOW_MAP_SIZE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SHADOW_SAMPLES :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_SHADOW_SAMPLES) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SHADOW_RANGE :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_SHADOW_RANGE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SHADOW_FILTER :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_SHADOW_FILTER) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case AMBIENT_LIGHT :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_AMBIENT_LIGHT) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case O_CONSTS :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_O_CONSTS) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case BIT_MAP :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_BIT_MAP) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SOLID_BGND :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_SOLID_BGND) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case V_GRADIENT :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_V_GRADIENT) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case USE_BIT_MAP :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_USE_BIT_MAP) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case USE_SOLID_BGND :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_USE_SOLID_BGND) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case USE_V_GRADIENT :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_USE_V_GRADIENT) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case FOG :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_FOG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case FOG_BGND :
                    state = PARSE_ERROR;
                    break;
                case DISTANCE_CUE :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_DISTANCE_CUE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case DCUE_BGND :
                    state = PARSE_ERROR;
                    break;
                case USE_FOG :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_USE_FOG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case USE_DISTANCE_CUE :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_USE_DISTANCE_CUE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case DEFAULT_VIEW :
                    if (parent->id_tag != MDATA
                          || (parent->flags & GOT_DEFAULT_VIEW) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case VIEW_TOP :          /*  fall through .....  */
                case VIEW_BOTTOM :
                case VIEW_LEFT :
                case VIEW_RIGHT :
                case VIEW_FRONT :
                case VIEW_BACK :
                case VIEW_USER :
                case VIEW_CAMERA :       /*  ..... until here  */
                    state = PARSE_ERROR;
                    break;
                case MAT_ENTRY :
                    InitialiseMatEntry(&(top->data.mat_entry));
                    break;
                case MAT_NAME :
                    top->data.string = NULL;
                    if (parent->id_tag == MAT_ENTRY
                          && (parent->flags & GOT_MAT_NAME) == 0) {
                        char buffer[17];

                        if (ReadString(stream,top,17,buffer)
                              && LookupMaterial(buffer,material_list)
                                   == NULL) {
                            top->data.string = BrMemStrDup(buffer);
                            if (top->data.string == NULL) {
                                state = OUT_OF_MEMORY;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_AMBIENT :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_AMBIENT) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_DIFFUSE :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_DIFFUSE) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SPECULAR :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_SPECULAR) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SHININESS :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_SHININESS) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_TRANSPARENCY :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_TRANSPARENCY) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_XPFALL :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_XPFALL) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_USE_XPFALL :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_USE_XPFALL) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_REFBLUR :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_REFBLUR) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_USE_REFBLUR :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_USE_REFBLUR) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SELF_ILLUM :
                    if (parent->id_tag != MAT_ENTRY
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_TWO_SIDE :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_TWO_SIDE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_DECAL :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_DECAL) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_ADDITIVE :
                    if (parent->id_tag != MAT_ENTRY
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SHADING :
                    if (parent->id_tag == MAT_ENTRY
                          && (parent->flags & GOT_MAT_SHADING) == 0) {
                        br_int_16 shading;

                        if (ReadInt16(stream,&shading)) {
#if TEST_WIREFRAME
                            shading -= 1;
#endif
                            top->done += sizeof(br_int_16);
                            switch (shading) {
                                case 0  : top->data.mat_shading = WIREFRAME;
                                          break;
                                case 1  : top->data.mat_shading = FLAT;
                                          break;
                                default : top->data.mat_shading = GOURAUD;
                                          break;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_TEXMAP :
                    InitialisePixmapRef(&(top->data.pixmap_ref));
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_TEXMAP) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SXP_TEXT_DATA :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_SXP_TEXT_DATA) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_OPACMAP :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_OPACMAP) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SXP_OPAC_DATA :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_SXP_OPAC_DATA) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_REFLMAP :
                    InitialisePixmapRef(&(top->data.pixmap_ref));
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_REFLMAP) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_ACUBIC :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_ACUBIC) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_BUMPMAP :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_BUMPMAP) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MAT_SXP_BUMP_DATA :
                    if (parent->id_tag != MAT_ENTRY
                          || (parent->flags & GOT_MAT_SXP_BUMP_DATA) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case NAMED_OBJECT :
                    top->data.named_obj = NULL;
                    if (parent->id_tag == MDATA) {
                        char buffer[12];

                        if (ReadString(stream,top,12,buffer)
                              && LookupNamedObj(buffer,named_objs) == NULL) {
                            top->data.named_obj = AllocateNamedObj(buffer);
                            if (top->data.named_obj == NULL) {
                                state = OUT_OF_MEMORY;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case N_TRI_OBJECT :
                    InitialiseNTriObj(&(top->data.n_tri_obj));
                    if (parent->id_tag == NAMED_OBJECT
                          && (parent->flags & GOT_N_TRI_OBJECT) == 0
                          && (parent->flags & GOT_N_DIRECT_LIGHT) == 0
                          && (parent->flags & GOT_N_CAMERA) == 0) {
                          if (options->log != NULL) {
                              fprintf(options->log,"Triangle mesh: \"%s\"\n",
                                      parent->data.named_obj->name);
                          }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case POINT_ARRAY :
                    InitialisePointArray(&(top->data.point_array));
                    if (parent->id_tag == N_TRI_OBJECT
                          && (parent->flags & GOT_POINT_ARRAY) == 0) {
                        state = ReadPointArray(stream,top,options);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case POINT_FLAG_ARRAY :
                    if (parent->id_tag != N_TRI_OBJECT
                          || (parent->flags & GOT_POINT_FLAG_ARRAY) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case FACE_ARRAY :
                    InitialiseFaceArray(&(top->data.face_array));
                    if (parent->id_tag == N_TRI_OBJECT
                          && (parent->flags & GOT_FACE_ARRAY) == 0) {
                        state = ReadFaceArray(stream,top);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MSH_MAT_GROUP :
                    top->data.msh_mat_group = NULL;
                    if (parent->id_tag == FACE_ARRAY) {
                        state = ReadMshMatGroup(stream,top,material_list);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case SMOOTH_GROUP :
                    top->data.smooth_group = NULL;
                    if (parent->id_tag == FACE_ARRAY
                          && (parent->flags & GOT_SMOOTH_GROUP) == 0) {
                        state = ReadSmoothGroup(stream,top,
                                          parent->data.face_array.n_faces,
                                          options);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case TEX_VERTS :
                    InitialiseTexVerts(&(top->data.tex_verts));
                    if (parent->id_tag == N_TRI_OBJECT
                          && (parent->flags & GOT_TEX_VERTS) == 0) {
                        state = ReadTexVerts(stream,top);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MESH_MATRIX :
                    if (parent->id_tag == N_TRI_OBJECT
                          && (parent->flags & GOT_MESH_MATRIX) == 0) {
                        state = ReadMeshMatrix(stream,top,options);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MESH_TEXTURE_INFO :
                    if (parent->id_tag != N_TRI_OBJECT
                          || (parent->flags & GOT_MESH_TEXTURE_INFO) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case PROC_NAME :
                    if (parent->id_tag != N_TRI_OBJECT
                          || (parent->flags & GOT_PROC_NAME) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case PROC_DATA :
                    if (parent->id_tag != N_TRI_OBJECT
                          || (parent->flags & GOT_PROC_DATA) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case N_DIRECT_LIGHT :
                    if (parent->id_tag == NAMED_OBJECT
                          && (parent->flags & GOT_N_TRI_OBJECT) == 0
                          && (parent->flags & GOT_N_DIRECT_LIGHT) == 0
                          && (parent->flags & GOT_N_CAMERA) == 0) {
                        state = ReadNDLight(stream,top,options);

                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case DL_OFF :
                    if (parent->id_tag == N_DIRECT_LIGHT
                          && (parent->flags & GOT_DL_OFF) == 0
                          && SkipRest(stream,top)) {
                        if (options->log != NULL) {
                            fprintf(options->log,"  light is turned off.\n");
                        }
                    } else {
                       state = PARSE_ERROR;
                    }
                    break;
                case DL_SPOTLIGHT :
                    if (parent->id_tag == N_DIRECT_LIGHT
                          && (parent->flags & GOT_DL_SPOTLIGHT) == 0) {
                        state = ReadDlSpotlight(stream,top,options);

                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case DL_SHADOWED :         /*  fall through .....  */
                case DL_LOCAL_SHADOW2 :
                case DL_SEE_CONE :         /*  ..... until here    */
                    state = PARSE_ERROR;
                    break;
                case N_CAMERA :
                    if (parent->id_tag == NAMED_OBJECT
                          && (parent->flags & GOT_N_TRI_OBJECT) == 0
                          && (parent->flags & GOT_N_DIRECT_LIGHT) == 0
                          && (parent->flags & GOT_N_CAMERA) == 0) {
                        state = ReadNCamera(stream,top,options);

                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case CAM_SEE_CONE :
                    state = PARSE_ERROR;
                    break;
                case OBJ_HIDDEN :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_HIDDEN) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_VIS_LOFTER :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_VIS_LOFTER) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_DOESNT_CAST :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_DOESNT_CAST) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_MATTE :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_MATTE) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_FAST :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_FAST) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_PROCEDURAL :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_PROCEDURAL) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJ_FROZEN :
                    if (parent->id_tag != NAMED_OBJECT
                          || (parent->flags & GOT_OBJ_FROZEN) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case KFDATA :
                    if (parent->id_tag != M3DMAGIC
                          || (parent->flags & GOT_KFDATA) != 0) {
                        state = PARSE_ERROR;
                    }
                    break;
                case KFHDR :
                    if (parent->id_tag == KFDATA
                          && (parent->flags & GOT_KFHDR) == 0) {
                        br_int_16 version;

                        if (ReadInt16(stream,&version)) {
                            top->done += sizeof(br_int_16);
                            if (options->verbose) {
                                if ((version == 1) || (version == 2)) {
                                    fprintf(stdout,"Keyframe data version "
                                            "%d.0.\n",version);
                                } else {
                                    fprintf(stdout,"Keyframe data version "
                                            "%d.0, only able to read "
                                            "versions 1.0 and 2.0 chunks.\n",
                                             version);
                                }
                            }
                            if (!SkipRest(stream,top)) {
                                state = PARSE_ERROR;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case KFSEG :
                    if (parent->id_tag != KFDATA
                          || (parent->flags & GOT_KFSEG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case KFCURTIME :
                    if (parent->id_tag != KFDATA
                          || (parent->flags & GOT_KFCURTIME) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case OBJECT_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag == KFDATA) {
                        top->data.node_tag = AllocateNodeTag();
                        if (top->data.node_tag == NULL) {
                            state = OUT_OF_MEMORY;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case CAMERA_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag == KFDATA) {
                        top->data.node_tag = AllocateNodeTag();
                        if (top->data.node_tag == NULL) {
                            state = OUT_OF_MEMORY;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case TARGET_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag != KFDATA
                         || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case LIGHT_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag == KFDATA) {
                        top->data.node_tag = AllocateNodeTag();
                        if (top->data.node_tag == NULL) {
                            state = OUT_OF_MEMORY;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case SPOTLIGHT_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag == KFDATA) {
                        top->data.node_tag = AllocateNodeTag();
                        if (top->data.node_tag == NULL) {
                            state = OUT_OF_MEMORY;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case L_TARGET_NODE_TAG :
                    top->data.node_tag = NULL;
                    if (parent->id_tag != KFDATA
                         || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case NODE_HDR :
                    InitialiseNodeHdr(&(top->data.node_hdr));
                    if ((parent->id_tag == OBJECT_NODE_TAG
                            || parent->id_tag == CAMERA_NODE_TAG
                            || parent->id_tag == LIGHT_NODE_TAG
                            || parent->id_tag == SPOTLIGHT_NODE_TAG)
                          && (parent->flags & GOT_NODE_HDR) == 0) {
                        state = ReadNodeHdr(stream,top,n_node_tags,named_objs);
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case PIVOT :
                    if (parent->id_tag == OBJECT_NODE_TAG
                          && (parent->flags & GOT_PIVOT) == 0) {
                        if (ReadPoint(stream,top,&(top->data.pivot), options)) {
#if SHOW_KEYFRAME_DATA
                            fprintf(stdout,"  PIVOT: (%f  %f  %f)\n",
                                    BrScalarToFloat(top->data.pivot.v[0]),
                                    BrScalarToFloat(top->data.pivot.v[1]),
                                    BrScalarToFloat(top->data.pivot.v[2]));
#endif
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case INSTANCE_NAME :
                    top->data.string = NULL;
                    if (parent->id_tag == OBJECT_NODE_TAG
                          && (parent->flags & GOT_INSTANCE_NAME) == 0) {
                        char buffer[12];

                        if (ReadString(stream,top,12,buffer)) {
#if SHOW_KEYFRAME_DATA
                            fprintf(stdout,"  INSTANCE_NAME: \"%s\"\n",buffer);
#endif
                            if (parent->data.named_obj != NULL) {
                                top->data.string = BrMemStrDup(buffer);
                                if (top->data.string == NULL) {
                                    state = OUT_OF_MEMORY;
                                }
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case MORPH_SMOOTH :
                    if (parent->id_tag != OBJECT_NODE_TAG
                          || (parent->flags & GOT_MORPH_SMOOTH) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case BOUNDBOX :
                    if (parent->id_tag == OBJECT_NODE_TAG
                          && (parent->flags & GOT_BOUNDBOX) == 0) {
                        br_vector3 *min;
                        br_vector3 *max;

                        min = &(top->data.boundbox.min);
                        max = &(top->data.boundbox.max);

                        if (ReadPoint(stream,top,min, options)
                              && ReadPoint(stream,top,max, options)) {
#if SHOW_KEYFRAME_DATA
                            fprintf(stdout,"  BOUNDBOX:\n"
                                           "    min: (%f  %f  %f)\n"
                                           "    max: (%f  %f  %f)\n",
                                    BrScalarToFloat(min->v[0]),
                                    BrScalarToFloat(min->v[1]),
                                    BrScalarToFloat(min->v[2]),
                                    BrScalarToFloat(max->v[0]),
                                    BrScalarToFloat(max->v[1]),
                                    BrScalarToFloat(max->v[2]));
#endif
                        } else {
                            state = PARSE_ERROR;
                        }
                    } else {
                        state = PARSE_ERROR;
                    }
                    break;
                case POS_TRACK_TAG :
                    if ((parent->id_tag != OBJECT_NODE_TAG
                            && parent->id_tag == CAMERA_NODE_TAG
                            && parent->id_tag == LIGHT_NODE_TAG
                            && parent->id_tag == SPOTLIGHT_NODE_TAG)
                          || (parent->flags & GOT_POS_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case COL_TRACK_TAG :
                    if ((parent->id_tag != SPOTLIGHT_NODE_TAG
                             && parent->id_tag != LIGHT_NODE_TAG)
                          || (parent->flags & GOT_COL_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case ROT_TRACK_TAG :
                    if (parent->id_tag != OBJECT_NODE_TAG
                          || (parent->flags & GOT_ROT_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case SCL_TRACK_TAG :
                    if (parent->id_tag != OBJECT_NODE_TAG
                          || (parent->flags & GOT_SCL_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case MORPH_TRACK_TAG :
                    if (parent->id_tag != OBJECT_NODE_TAG
                          || (parent->flags & GOT_MORPH_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case FOV_TRACK_TAG :
                    if (parent->id_tag != CAMERA_NODE_TAG
                          || (parent->flags & GOT_FOV_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case ROLL_TRACK_TAG :
                    if ((parent->id_tag != CAMERA_NODE_TAG
                             && parent->id_tag != SPOTLIGHT_NODE_TAG)
                          || (parent->flags & GOT_ROLL_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case HOT_TRACK_TAG :
                    if (parent->id_tag != SPOTLIGHT_NODE_TAG
                          || (parent->flags & GOT_HOT_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                case FALL_TRACK_TAG :
                    if (parent->id_tag != SPOTLIGHT_NODE_TAG
                          || (parent->flags & GOT_FALL_TRACK_TAG) != 0
                          || !SkipRest(stream,top)) {
                        state = PARSE_ERROR;
                    }
                    break;
                default :
                    if (!SkipRest(stream,top)) { /* skip unidentified chunks */
                        state = PARSE_ERROR;
                    }
                    break;
             }
#if SHOW_BROKEN_CHUNKS
             if (state != OK) {
                 DisplayTop("Failed PUSH",top,stack);
             }
#endif
         }
         while (top >= stack && (state != OK || top->done == top->length)) {
#if SHOW_STACK
            DisplayTop("POP",top,stack);
#endif
            switch (top->id_tag) {
                case COLOR_F :
                    if (state == OK) {
                        if (parent->id_tag == N_DIRECT_LIGHT) {
                            BlockCopy(&(parent->data.n_d_light.color),
                                      &(top->data.color),
                                      sizeof(Color_t));
                        } else {
                            BlockCopy(&(parent->data.color),
                                      &(top->data.color),
                                      sizeof(Color_t));
                        }
                        parent->flags |= GOT_COLOR_F;
                    }
                    break;
                case COLOR_24 :
                    if (state == OK) {
                        if (parent->id_tag == N_DIRECT_LIGHT) {
                            BlockCopy(&(parent->data.n_d_light.color),
                                      &(top->data.color),
                                      sizeof(Color_t));
                        } else {
                            BlockCopy(&(parent->data.color),
                                      &(top->data.color),
                                      sizeof(Color_t));
                        }
                        parent->flags |= GOT_COLOR_24;
                    }
                    break;
                case INT_PERCENTAGE :
                    if (state == OK) {
                        if (parent->id_tag == MAT_TEXMAP
                              || parent->id_tag == MAT_REFLMAP) {
                            parent->data.pixmap_ref.strength
                                 = top->data.percent;
                        } else {
                            parent->data.percent = top->data.percent;
                        }
                        parent->flags |= GOT_INT_PERCENTAGE;
                    }
                    break;
                case FLOAT_PERCENTAGE :
                    if (state == OK) {
                        if (parent->id_tag == MAT_TEXMAP
                              || parent->id_tag == MAT_REFLMAP) {
                            parent->data.pixmap_ref.strength
                                 = top->data.percent;
                        } else {
                            parent->data.percent = top->data.percent;
                        }
                        parent->flags |= GOT_FLOAT_PERCENTAGE;
                    }
                    break;
                case MAT_MAPNAME :
                    if (state == OK) {
                        parent->data.pixmap_ref.mat_mapname = top->data.string;
                        parent->flags |= GOT_MAT_MAPNAME;
                    }
                    break;

                case M3DMAGIC :
                    break;

                case M3D_VERSION :
                    if (state == OK) {
                        parent->flags |= GOT_M3D_VERSION;
                    }
                    break;
                case MDATA :
                    if (state == OK) {
                        parent->flags |= GOT_MDATA;
                    }
                    break;
                case MESH_VERSION :
                    if (state == OK) {
                        parent->flags |= GOT_MESH_VERSION;
                    }
                    break;
                case MASTER_SCALE :
                    if (state == OK) {
                        parent->flags |= GOT_MASTER_SCALE;
                    }
                    break;
                case LO_SHADOW_BIAS :
                    if (state == OK) {
                        parent->flags |= GOT_LO_SHADOW_BIAS;
                    }
                    break;
                case HI_SHADOW_BIAS :
                    if (state == OK) {
                        parent->flags |= GOT_HI_SHADOW_BIAS;
                    }
                    break;
                case SHADOW_MAP_SIZE :
                    if (state == OK) {
                        parent->flags |= GOT_SHADOW_MAP_SIZE;
                    }
                    break;
                case SHADOW_SAMPLES :
                    if (state == OK) {
                        parent->flags |= GOT_SHADOW_SAMPLES;
                    }
                    break;
                case SHADOW_RANGE :
                    if (state == OK) {
                        parent->flags |= GOT_SHADOW_RANGE;
                    }
                    break;
                case SHADOW_FILTER :
                    if (state == OK) {
                        parent->flags |= GOT_SHADOW_FILTER;
                    }
                    break;
                case AMBIENT_LIGHT :
                    if (state == OK) {
                        parent->flags |= GOT_AMBIENT_LIGHT;
                    }
                    break;
                case O_CONSTS :
                    if (state == OK) {
                        parent->flags |= GOT_O_CONSTS;
                    }
                    break;
                case BIT_MAP :
                    if (state == OK) {
                        parent->flags |= GOT_BIT_MAP;
                    }
                    break;
                case SOLID_BGND :
                    if (state == OK) {
                        parent->flags |= GOT_SOLID_BGND;
                    }
                    break;
                case V_GRADIENT :
                    if (state == OK) {
                        parent->flags |= GOT_V_GRADIENT;
                    }
                    break;
                case USE_BIT_MAP :
                    if (state == OK) {
                        parent->flags |= GOT_USE_BIT_MAP;
                    }
                    break;
                case USE_SOLID_BGND :
                    if (state == OK) {
                        parent->flags |= GOT_USE_SOLID_BGND;
                    }
                    break;
                case USE_V_GRADIENT :
                    if (state == OK) {
                        parent->flags |= GOT_USE_V_GRADIENT;
                    }
                    break;
                case FOG :
                    if (state == OK) {
                        parent->flags |= GOT_FOG;
                    }
                    break;
                case FOG_BGND :
                    break;
                case DISTANCE_CUE :
                    if (state == OK) {
                        parent->flags |= GOT_DISTANCE_CUE;
                    }
                    break;
                case DCUE_BGND :
                    break;
                case USE_FOG :
                    if (state == OK) {
                        parent->flags |= GOT_USE_FOG;
                    }
                    break;
                case USE_DISTANCE_CUE :
                    if (state == OK) {
                        parent->flags |= GOT_USE_DISTANCE_CUE;
                    }
                    break;
                case DEFAULT_VIEW :
                    if (state == OK) {
                        parent->flags |= GOT_DEFAULT_VIEW;
                    }
                    break;
                case VIEW_TOP :               /*  fall through .....  */
                case VIEW_BOTTOM :
                case VIEW_LEFT :
                case VIEW_RIGHT :
                case VIEW_FRONT :
                case VIEW_BACK :
                case VIEW_USER :
                case VIEW_CAMERA :            /*  ..... until here    */
                    break;
                case MAT_ENTRY :
                    if (state == OK) {
                        if (top->flags & GOT_MAT_NAME) {
                            MaterialList_t *material_link;

                            material_link = ConvertMaterial(
                                                       &(top->data.mat_entry),
                                                       &pixmap_list,options);

                            if (material_link != NULL) {
                                material_link->next = material_list;
                                material_list       = material_link;
                            } else {
                                state = OUT_OF_MEMORY;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    DismantleMatEntry(&(top->data.mat_entry));
                    break;
                case MAT_NAME :
                    if (state == OK) {
                        parent->data.mat_entry.mat_name = top->data.string;
                        parent->flags |= GOT_MAT_NAME;
                    }
                    break;
                case MAT_AMBIENT :
                    if (state == OK) {
                        if ((top->flags & GOT_COLOR_F) != 0
                              || (top->flags & GOT_COLOR_24) != 0) {
                            BlockCopy(&(parent->data.mat_entry.mat_ambient),
                                      &(top->data.color),
                                      sizeof(Color_t));
                            parent->flags |= GOT_MAT_AMBIENT;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case MAT_DIFFUSE :
                    if (state == OK) {
                        if ((top->flags & GOT_COLOR_F) != 0
                              || (top->flags & GOT_COLOR_24) != 0) {
                            BlockCopy(&(parent->data.mat_entry.mat_diffuse),
                                      &(top->data.color),
                                      sizeof(Color_t));
                            parent->flags |= GOT_MAT_DIFFUSE;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case MAT_SPECULAR :
                    if (state == OK) {
                        if ((top->flags & GOT_COLOR_F) != 0
                              || (top->flags & GOT_COLOR_24) != 0) {
                            BlockCopy(&(parent->data.mat_entry.mat_specular),
                                      &(top->data.color),
                                      sizeof(Color_t));
                            parent->flags |= GOT_MAT_SPECULAR;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case MAT_SHININESS :
                    if (state == OK) {
                        if ((top->flags & GOT_FLOAT_PERCENTAGE) != 0
                              || (top->flags & GOT_INT_PERCENTAGE) != 0) {
                            parent->data.mat_entry.mat_shininess =
                                 top->data.percent;
                            parent->flags |= GOT_MAT_SHININESS;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case MAT_TRANSPARENCY :
                    if (state == OK) {
                        if ((top->flags & GOT_FLOAT_PERCENTAGE) != 0
                              || (top->flags & GOT_INT_PERCENTAGE) != 0) {
                            parent->data.mat_entry.mat_transparency
                                 = top->data.percent;
                            parent->flags |= GOT_MAT_TRANSPARENCY;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case MAT_XPFALL :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_XPFALL;
                    }
                    break;
                case MAT_USE_XPFALL :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_USE_XPFALL;
                    }
                    break;
                case MAT_REFBLUR :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_REFBLUR;
                    }
                    break;
                case MAT_USE_REFBLUR :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_USE_REFBLUR;
                    }
                    break;
                case MAT_SELF_ILLUM :
                    break;
                case MAT_TWO_SIDE :
                    if (state == OK) {
                        parent->data.mat_entry.mat_two_side = TRUE;
                        parent->flags |= GOT_MAT_TWO_SIDE;
                    }
                    break;
                case MAT_DECAL :
                    if (state == OK) {
                        parent->data.mat_entry.mat_decal = TRUE;
                        parent->flags |= GOT_MAT_DECAL;
                    }
                    break;
                case MAT_ADDITIVE :
                    break;
                case MAT_SHADING :
                    if (state == OK) {
                        parent->data.mat_entry.mat_shading
                                   = top->data.mat_shading;
                        parent->flags |= GOT_MAT_SHADING;
                    }
                    break;
                case MAT_TEXMAP :
                    if (state == OK) {
                        if ((top->flags & GOT_MAT_MAPNAME) != 0
                             && ((top->flags & GOT_FLOAT_PERCENTAGE) != 0
                                  || (top->flags & GOT_INT_PERCENTAGE) != 0)) {
                            UpdatePixmapRef(
                                     &(parent->data.mat_entry.pixmap_ref),
                                     &(top->data.pixmap_ref));

                            parent->flags |= GOT_MAT_TEXMAP;

                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK) {
                        DismantlePixmapRef(&(top->data.pixmap_ref));
                    }
                    break;
                case MAT_SXP_TEXT_DATA :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_SXP_TEXT_DATA;
                    }
                    break;
                case MAT_OPACMAP :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_OPACMAP;
                    }
                    break;
                case MAT_SXP_OPAC_DATA :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_SXP_OPAC_DATA;
                    }
                    break;
                case MAT_REFLMAP :
                    if (state == OK) {
                        if ((top->flags & GOT_MAT_MAPNAME) != 0
                             && ((top->flags & GOT_FLOAT_PERCENTAGE) != 0
                                  || (top->flags & GOT_INT_PERCENTAGE) != 0)) {
							top->data.pixmap_ref.is_reflection_map = TRUE;
							UpdatePixmapRef(
								&(parent->data.mat_entry.pixmap_ref),
								&(top->data.pixmap_ref));
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK) {
                        DismantlePixmapRef(&(top->data.pixmap_ref));
                    }
                    break;
                case MAT_ACUBIC :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_ACUBIC;
                    }
                    break;
                case MAT_BUMPMAP :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_BUMPMAP;
                    }
                    break;
                case MAT_SXP_BUMP_DATA :
                    if (state == OK) {
                        parent->flags |= GOT_MAT_SXP_BUMP_DATA;
                    }
                    break;
                case NAMED_OBJECT :
                    if (state == OK) {
                        if ((top->flags & GOT_N_CAMERA) != 0
                              || (top->flags & GOT_N_DIRECT_LIGHT) != 0
                              || (top->flags & GOT_N_TRI_OBJECT) != 0) {

                            top->data.named_obj->next = named_objs;
                            named_objs = top->data.named_obj;

                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK) {
                        DeallocateNamedObjs(top->data.named_obj);
                    }
                    break;
                case N_TRI_OBJECT :
                    if (state == OK) {
                        if ((top->flags & GOT_POINT_ARRAY) != 0
                               && (top->flags & GOT_FACE_ARRAY) != 0) {
                            state = ConvertNTriObj(&(top->data.n_tri_obj),
                                                   parent->data.named_obj,
                                                   options);
                            if (state == OK) {
                                parent->flags |= GOT_N_TRI_OBJECT;
                            }
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    DismantleNTriObj(&(top->data.n_tri_obj));
                    break;
                case POINT_ARRAY :
                    if (state == OK) {
                        if (options->log != NULL) {
                            fprintf(options->log,"  %d vertices\n",
                                    top->data.point_array.n_vertices);
                        }
                        BlockCopy(&(parent->data.n_tri_obj.point_array),
                                  &(top->data.point_array),
                                  sizeof(PointArray_t));


                        parent->flags |= GOT_POINT_ARRAY;
                    } else {
                        DismantlePointArray(&(top->data.point_array));
                    }
                    break;
                case POINT_FLAG_ARRAY :
                    if (state == OK) {
                        parent->flags |= GOT_POINT_FLAG_ARRAY;
                    }
                    break;
                case FACE_ARRAY :
                    if (state == OK) {
                        if (options->log != NULL) {
                            fprintf(options->log,"  %d faces\n",
                                    top->data.face_array.n_faces);
                        }
                        BlockCopy(&(parent->data.n_tri_obj.face_array),
                                  &(top->data.face_array),
                                  sizeof(FaceArray_t));
                        parent->flags |= GOT_FACE_ARRAY;
                    } else {
                        DismantleFaceArray(&(top->data.face_array));
                    }
                    break;
                case MSH_MAT_GROUP :
                    if (state == OK) {
                        top->data.msh_mat_group->next
                             = parent->data.face_array.msh_mat_groups;
                        parent->data.face_array.msh_mat_groups
                             = top->data.msh_mat_group;
                    } else {
                        DeallocateMshMatGroups(top->data.msh_mat_group);
                    }
                    break;
                case SMOOTH_GROUP :
                    if (state == OK) {
                        parent->data.face_array.smooth_group
                             = top->data.smooth_group;
                        parent->flags |= GOT_SMOOTH_GROUP;
                    } else if (top->data.smooth_group != NULL) {
                        BrMemFree(top->data.smooth_group);
                    }
                    break;
                case TEX_VERTS :
                    if (state == OK) {
                        if (options->log != NULL) {
                            fprintf(options->log,"  %d texture vertices\n",
                                    top->data.tex_verts.n_texverts);
                        }
                        BlockCopy(&(parent->data.n_tri_obj.tex_verts),
                                  &(top->data.tex_verts),
                                  sizeof(TexVerts_t));
                        parent->flags |= GOT_TEX_VERTS;
                    } else {
                        DismantleTexVerts(&(top->data.tex_verts));
                    }
                    break;
                case MESH_MATRIX :
                    if (state == OK) {
                        BrMatrix34Copy(&(parent->data.n_tri_obj.mesh_matrix),
                                       &(top->data.mesh_matrix));
                        parent->flags |= GOT_MESH_MATRIX;
                    }
                    break;
                case MESH_TEXTURE_INFO :
                    if (state == OK) {
                        parent->flags |= GOT_MESH_TEXTURE_INFO;
                    }
                    break;
                case PROC_NAME :
                    if (state == OK) {
                        parent->flags |= GOT_PROC_NAME;
                    }
                    break;
                case PROC_DATA :
                    if (state == OK) {
                        parent->flags |= GOT_PROC_DATA;
                    }
                    break;
                case N_DIRECT_LIGHT :
                    if (state == OK) {
                        if ((top->flags & GOT_COLOR_F) != 0
                               || (top->flags & GOT_COLOR_24) != 0) {
                            ConvertNDLight(&(top->data.n_d_light),
                                           parent->data.named_obj,
                                           options);
                            parent->flags |= GOT_N_DIRECT_LIGHT;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    break;
                case DL_OFF :
                    if (state == OK) {
                        parent->data.n_d_light.is_off = TRUE;

                        parent->flags |= GOT_DL_OFF;
                    }
                    break;
                case DL_SPOTLIGHT :
                    if (state == OK) {
                        parent->data.n_d_light.is_spotlight = TRUE;
                        BlockCopy(&(parent->data.n_d_light.dl_spotlight),
                                  &(top->data.dl_spotlight),
                                  sizeof(DlSpotlight_t));

                        parent->flags |= GOT_DL_SPOTLIGHT;
                    }
                    break;
                case DL_SHADOWED :         /*  fall through .....  */
                case DL_LOCAL_SHADOW2 :
                case DL_SEE_CONE :         /*  ..... until here    */
                    break;
                case N_CAMERA :
                    if (state == OK) {
                        ConvertNCamera(&(top->data.n_camera),
                                       parent->data.named_obj,
                                       options);
                        parent->flags |= GOT_N_CAMERA;
                    }
                    break;
                case CAM_SEE_CONE :
                    state = PARSE_ERROR;
                    break;
                case OBJ_HIDDEN :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_HIDDEN;
                    }
                    break;
                case OBJ_VIS_LOFTER :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_VIS_LOFTER;
                    }
                    break;
                case OBJ_DOESNT_CAST :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_DOESNT_CAST;
                    }
                    break;
                case OBJ_MATTE :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_MATTE;
                    }
                    break;
                case OBJ_FAST :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_FAST;
                    }
                    break;
                case OBJ_PROCEDURAL :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_PROCEDURAL;
                    }
                    break;
                case OBJ_FROZEN :
                    if (state == OK) {
                        parent->flags |= GOT_OBJ_FROZEN;
                    }
                    break;
                case KFDATA :
                    if (state == OK) {
                        if ((top->flags & GOT_KFHDR) != 0) {
                             parent->flags |= GOT_KFDATA;
                        } else {
                             state = PARSE_ERROR;
                        }
                    }
                    break;
                case KFHDR :
                    if (state == OK) {
                        parent->flags |= GOT_KFHDR;
                    }
                    break;
                case KFSEG :
                    if (state == OK) {
                        parent->flags |= GOT_KFSEG;
                    }
                    break;
                case KFCURTIME :
                    if (state == OK) {
                        parent->flags |= GOT_KFCURTIME;
                    }
                    break;
                case OBJECT_NODE_TAG :
                    if (state == OK) {
                        if ((top->flags & GOT_NODE_HDR) != 0
                              && (top->flags & GOT_PIVOT) != 0
                              && (top->flags & GOT_POS_TRACK_TAG) != 0
                              && (top->flags & GOT_ROT_TRACK_TAG) != 0
                              && (top->flags & GOT_SCL_TRACK_TAG) != 0
                              && InsertNodeTag(top->data.node_tag,
                                               &node_tags)) {
                            n_node_tags += 1;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK && top->data.node_tag != NULL) {
                        DeallocateNodeTags(top->data.node_tag);
                    }
                    break;
                case CAMERA_NODE_TAG :
                    if (state == OK) {
                        if ((top->flags & GOT_NODE_HDR) != 0
                              && (top->flags & GOT_POS_TRACK_TAG) != 0
                              && (top->flags & GOT_FOV_TRACK_TAG) != 0
                              && (top->flags & GOT_ROLL_TRACK_TAG) != 0
                              && InsertNodeTag(top->data.node_tag,
                                               &node_tags)) {
                            n_node_tags += 1;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK && top->data.node_tag != NULL) {
                        DeallocateNodeTags(top->data.node_tag);
                    }
                    break;
                case TARGET_NODE_TAG :
                    if (state == OK) {
                        n_node_tags += 1;
                    }
                    break;
                case LIGHT_NODE_TAG :
                    if (state == OK) {
                        if ((top->flags & GOT_NODE_HDR) != 0
                              && (top->flags & GOT_POS_TRACK_TAG) != 0
                              && (top->flags & GOT_COL_TRACK_TAG) != 0
                              && InsertNodeTag(top->data.node_tag,
                                               &node_tags)) {
                            n_node_tags += 1;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK && top->data.node_tag != NULL) {
                        DeallocateNodeTags(top->data.node_tag);
                    }
                    break;
                case SPOTLIGHT_NODE_TAG :
                    if (state == OK) {
                        if ((top->flags & GOT_NODE_HDR) != 0
                              && (top->flags & GOT_POS_TRACK_TAG) != 0
                              && (top->flags & GOT_COL_TRACK_TAG) != 0
                              && (top->flags & GOT_HOT_TRACK_TAG) != 0
                              && (top->flags & GOT_FALL_TRACK_TAG) != 0
                              && InsertNodeTag(top->data.node_tag,
                                               &node_tags)) {
                            n_node_tags += 1;
                        } else {
                            state = PARSE_ERROR;
                        }
                    }
                    if (state != OK && top->data.node_tag != NULL) {
                        DeallocateNodeTags(top->data.node_tag);
                    }
                    break;
                case L_TARGET_NODE_TAG :
                    if (state == OK) {
                        n_node_tags += 1;
                    }
                    break;
                case NODE_HDR :
                    if (state == OK) {
                        BlockCopy(&(parent->data.node_tag->node_hdr),
                                  &(top->data.node_hdr),
                                  sizeof(NodeHdr_t));

                        parent->flags |= GOT_NODE_HDR;
                    }
                    break;
                case PIVOT :
                    if (state == OK) {
                        parent->data.node_tag->has_pivot = TRUE;
                        BrVector3Copy(&(parent->data.node_tag->pivot),
                                      &(top->data.pivot));

                        parent->flags |= GOT_PIVOT;
                    }
                    break;
                case INSTANCE_NAME :
                    if (state == OK) {
                        parent->data.node_tag->instance_name
                                         = top->data.string;

                        parent->flags |= GOT_INSTANCE_NAME;
                    }
                    if (state != OK && top->data.string != NULL) {
                        BrMemFree(top->data.string);
                    }
                    break;
                case MORPH_SMOOTH :
                    if (state == OK) {
                        parent->flags |= GOT_MORPH_SMOOTH;
                    }
                    break;
                case BOUNDBOX :
                    if (state == OK) {
                        parent->data.node_tag->has_boundbox = TRUE;
                        BrVector3Copy(&(parent->data.node_tag->boundbox.min),
                                      &(top->data.boundbox.min));
                        BrVector3Copy(&(parent->data.node_tag->boundbox.max),
                                      &(top->data.boundbox.max));

                        parent->flags |= GOT_BOUNDBOX;
                    }
                    break;
                case POS_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_POS_TRACK_TAG;
                    }
                    break;
                case COL_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_COL_TRACK_TAG;
                    }
                    break;
                case ROT_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_ROT_TRACK_TAG;
                    }
                    break;
                case SCL_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_SCL_TRACK_TAG;
                    }
                    break;
                case MORPH_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_MORPH_TRACK_TAG;
                    }
                    break;
                case FOV_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_FOV_TRACK_TAG;
                    }
                    break;
                case ROLL_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_ROLL_TRACK_TAG;
                    }
                    break;
                case HOT_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_HOT_TRACK_TAG;
                    }
                    break;
                case FALL_TRACK_TAG :
                    if (state == OK) {
                        parent->flags |= GOT_FALL_TRACK_TAG;
                    }
                    break;
                default :
                    break;
            }
            parent->done += top->done;

#if SHOW_BROKEN_CHUNKS
            if (state != OK) {
                DisplayTop("Failed POP",top,stack);
            }
#endif

            top = parent--;                     /* POP */

        }
    }

	/*
	 * Finished parsing file
	 */
    if (stream != NULL) {
        CloseBinInStream(stream);
        DeallocateBinInStream(stream);
    }

    if (state == OK && (options->mat_filename != NULL ||
						options->scr_filename != NULL)) {
        all_materials = CollectMaterials(material_list,&n_materials);

		if(all_materials == NULL)
                state = OUT_OF_MEMORY;
	}

	if ((n_materials > 0) && options->mat_filename) {
		filename = options->mat_filename;
		n_saved  = BrMaterialSaveMany(filename,all_materials,
                                       n_materials);

		if (n_saved != n_materials) {
		    fprintf(stdout,"Could only save %d out of %d "
				"materials in \"%s\".\n",n_saved,n_materials,
				filename);
		} else if (options->log != NULL) {
			fprintf(options->log,"Saved all %d materials in \"%s\".\n",
				n_materials,filename);
		}

		if (options->verbose && options->log == NULL) {
			fprintf(stdout,"Saved all %d materials in \"%s\".\n",
			n_materials,filename);
		}
    }

	if ((n_materials > 0) && options->scr_filename) {
		filename = options->scr_filename;
		n_saved  = BrFmtScriptMaterialSaveMany(filename,all_materials,
                                       n_materials);

		if (n_saved != n_materials) {
		    fprintf(stdout,"Could only save %d out of %d "
				"materials in \"%s\".\n",n_saved,n_materials,
				filename);
		} else if (options->log != NULL) {
			fprintf(options->log,"Saved all %d materials in \"%s\".\n",
				n_materials,filename);
		}

		if (options->verbose && options->log == NULL) {
			fprintf(stdout,"Saved all %d materials in \"%s\".\n",
			n_materials,filename);
		}
    }

	if(all_materials)
		BrMemFree(all_materials);

    if (state == OK && options->act_filename != NULL) {

        if (n_node_tags == 0 || options->flat_hierarchy) {
            world = BuildFlatHierarchy(named_objs, options);
        } else {
            world = BuildComplexHierarchy(node_tags, options);
        }

        if (world != NULL) {
            filename = options->act_filename;
            n_saved  = BrActorSave(filename,world);

            if (n_saved == 0) {
                fprintf(stdout,"Failed to save actor hierarchy in \"%s\".\n",
                        filename);
            } else if (options->log != NULL) {
                fprintf(options->log,"Saved actor hierarchy in \"%s\".\n",
                        filename);
            }
            if (options->verbose && options->log == NULL) {
                fprintf(stdout,"Saved actor hierarchy in \"%s\".\n",
                        filename);
            }
            BrActorFree(world);

        } else {
            state = OUT_OF_MEMORY;
        }
    }

    if (state == OK && options->mod_filename != NULL) {
        all_models = CollectModels(named_objs,&n_models);

        if (n_models > 0) {
            if (all_models != NULL) {
                filename = options->mod_filename;
                n_saved  = BrModelSaveMany(filename,all_models,n_models);

                if (n_saved != n_models) {
                    fprintf(stdout,"Could only save %d out of %d models "
                            "in \"%s\".\n",n_saved,n_models,filename);
                } else if (options->log != NULL) {
                    fprintf(options->log,"Saved all %d models in \"%s\".\n",
                            n_models,filename);
                }
                if (options->verbose && options->log == NULL) {
                    fprintf(stdout,"Saved all %d models in \"%s\".\n",
                            n_models,filename);
                }
                BrMemFree(all_models);

            } else {
               state = OUT_OF_MEMORY;
            }
        }
    }

    DeallocatePixmapList(pixmap_list);
    DeallocateMaterialList(material_list);
    DeallocateNamedObjs(named_objs);
    DeallocateNodeTags(node_tags);

    if (state == PARSE_ERROR) {
        fprintf(stdout,"\"%s\" is corrupt.\n",options->input_filename);
    } else if(state == OUT_OF_MEMORY) {
        fprintf(stdout,"Out of memory.\n");
    } else if (options->verbose) {
        fprintf(stdout,"Successfully parsed \"%s\".\n",
                options->input_filename);
    }
    return (state == OK);
}

#undef SHOW_BROKEN_CHUNKS

#undef RED_GREYSCALE_FACTOR
#undef GREEN_GREYSCALE_FACTOR
#undef BLUE_GREYSCALE_FACTOR

#undef FENCE
