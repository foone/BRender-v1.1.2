.386p
                NAME    vector
                EXTERNDEF   __8087 :BYTE
                EXTERNDEF   __init_387_emulator :BYTE
                EXTERNDEF   _BrFixedRLength3 :BYTE
                EXTERNDEF   _BrFixedLength3 :BYTE
                EXTERNDEF   _BrFixedLength2 :BYTE
                EXTERNDEF   __CHP :BYTE
DGROUP          GROUP   CONST,CONST2,_DATA,_BSS
_TEXT           SEGMENT PARA PUBLIC USE32 'CODE'
                ASSUME  CS:_TEXT ,DS:DGROUP,SS:DGROUP
                PUBLIC  _BrVector2Copy 
                PUBLIC  _BrVector2Set 
                PUBLIC  _BrVector2SetInt 
                PUBLIC  _BrVector2SetFloat 
                PUBLIC  _BrVector2Negate 
                PUBLIC  _BrVector2Add 
                PUBLIC  _BrVector2Accumulate 
                PUBLIC  _BrVector2Sub 
                PUBLIC  _BrVector2Scale 
                PUBLIC  _BrVector2InvScale 
                PUBLIC  _BrVector2Dot 
                PUBLIC  _BrVector2Length 
                PUBLIC  _BrVector2LengthSquared 
                PUBLIC  _BrVector3Copy 
                PUBLIC  _BrVector3Set 
                PUBLIC  _BrVector3SetInt 
                PUBLIC  _BrVector3SetFloat 
                PUBLIC  _BrVector3Negate 
                PUBLIC  _BrVector3Add 
                PUBLIC  _BrVector3Accumulate 
                PUBLIC  _BrVector3Sub 
                PUBLIC  _BrVector3Scale 
                PUBLIC  _BrVector3InvScale 
                PUBLIC  _BrVector3Dot 
                PUBLIC  _BrVector3Cross 
                PUBLIC  _BrVector3Length 
                PUBLIC  _BrVector3LengthSquared 
                PUBLIC  _BrVector3Normalise 
                PUBLIC  _BrVector3NormaliseLP 
                PUBLIC  _BrVector4Dot 
                PUBLIC  _BrVector4Copy 
                PUBLIC  BrFVector2Dot_ 
                PUBLIC  BrFVector3Copy_ 
                PUBLIC  BrVector3ScaleF_ 
                PUBLIC  BrFVector3Dot_ 
                PUBLIC  BrFVector3Normalise_ 
                PUBLIC  BrFVector3NormaliseLP_ 
                PUBLIC  _BrVector2Normalise 
_BrVector2Copy: mov     edx,dword ptr (+4H)[esp]
                mov     eax,dword ptr (+8H)[esp]
                mov     eax,dword ptr [eax]
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+8H)[esp]
                mov     eax,dword ptr (+4H)[eax]
                mov     dword ptr (+4H)[edx],eax
                ret     
                nop     
_BrVector2Set:  mov     eax,dword ptr (+4H)[esp]
                mov     edx,dword ptr (+8H)[esp]
                mov     dword ptr [eax],edx
                mov     edx,dword ptr (+0cH)[esp]
                mov     dword ptr (+4H)[eax],edx
                ret     
                mov     eax,eax
_BrVector2SetInt:
                mov     eax,dword ptr (+4H)[esp]
                mov     edx,dword ptr (+8H)[esp]
                shl     edx,10H
                mov     dword ptr [eax],edx
                mov     edx,dword ptr (+0cH)[esp]
                shl     edx,10H
                mov     dword ptr (+4H)[eax],edx
                ret     
_BrVector2SetFloat:
                mov     eax,dword ptr (+4H)[esp]
                fld     dword ptr L5
                fld     dword ptr (+8H)[esp]
                fmul    st,st(1)
                call    near ptr __CHP
                fistp   dword ptr [eax]
                fmul    dword ptr (+0cH)[esp]
                call    near ptr __CHP
                fistp   dword ptr (+4H)[eax]
                ret     
_BrVector2Negate:
                push    ebx
                mov     edx,dword ptr (+8H)[esp]
                mov     eax,dword ptr (+0cH)[esp]
                mov     eax,dword ptr [eax]
                mov     ebx,eax
                neg     ebx
                mov     eax,dword ptr (+0cH)[esp]
                mov     dword ptr [edx],ebx
                mov     eax,dword ptr (+4H)[eax]
                mov     ecx,eax
                neg     ecx
                mov     dword ptr (+4H)[edx],ecx
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
_BrVector2Add:  push    esi
                push    edi
                mov     edx,dword ptr (+0cH)[esp]
                mov     ecx,dword ptr (+14H)[esp]
                mov     eax,dword ptr (+10H)[esp]
                mov     esi,dword ptr [ecx]
                mov     eax,dword ptr [eax]
                add     eax,esi
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+10H)[esp]
                mov     edi,dword ptr (+4H)[ecx]
                mov     eax,dword ptr (+4H)[eax]
                add     eax,edi
                mov     dword ptr (+4H)[edx],eax
                pop     edi
                pop     esi
                ret     
_BrVector2Accumulate:
                push    ebx
                mov     eax,dword ptr (+8H)[esp]
                mov     edx,dword ptr (+0cH)[esp]
                mov     ebx,dword ptr [eax]
                mov     edx,dword ptr [edx]
                add     ebx,edx
                mov     dword ptr [eax],ebx
                mov     edx,dword ptr (+0cH)[esp]
                mov     ecx,dword ptr (+4H)[eax]
                mov     edx,dword ptr (+4H)[edx]
                add     ecx,edx
                mov     dword ptr (+4H)[eax],ecx
                pop     ebx
                ret     
                mov     eax,eax
_BrVector2Sub:  push    esi
                push    edi
                mov     edx,dword ptr (+0cH)[esp]
                mov     ecx,dword ptr (+14H)[esp]
                mov     eax,dword ptr (+10H)[esp]
                mov     esi,dword ptr [ecx]
                mov     eax,dword ptr [eax]
                sub     eax,esi
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+10H)[esp]
                mov     edi,dword ptr (+4H)[ecx]
                mov     eax,dword ptr (+4H)[eax]
                sub     eax,edi
                mov     dword ptr (+4H)[edx],eax
                pop     edi
                pop     esi
                ret     
_BrVector2Scale: push    ebx
                mov     ecx,dword ptr (+10H)[esp]
                mov     eax,dword ptr (+0cH)[esp]
                mov     ebx,ecx
                mov     eax,dword ptr [eax]
                imul    ebx
                shrd    eax,edx,10H
                mov     edx,dword ptr (+8H)[esp]
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+0cH)[esp]
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[eax]
                mov     ecx,dword ptr (+8H)[esp]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[ecx],eax
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
_BrVector2InvScale:
                push    ebx
                mov     ecx,dword ptr (+10H)[esp]
                mov     edx,dword ptr (+0cH)[esp]
                mov     ebx,ecx
                mov     edx,dword ptr [edx]
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     edx,dword ptr (+8H)[esp]
                mov     dword ptr [edx],eax
                mov     edx,dword ptr (+0cH)[esp]
                mov     ebx,ecx
                mov     edx,dword ptr (+4H)[edx]
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     edx,dword ptr (+8H)[esp]
                mov     dword ptr (+4H)[edx],eax
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
_BrVector2Dot:  push    ebx
                mov     eax,dword ptr (+8H)[esp]
                mov     edx,dword ptr (+0cH)[esp]
                mov     ebx,dword ptr (+4H)[edx]
                mov     ecx,dword ptr (+4H)[eax]
                mov     edx,dword ptr [edx]
                mov     eax,dword ptr [eax]
                imul    edx
                shrd    eax,edx,10H
                xchg    eax,ecx
                imul    ebx
                shrd    eax,edx,10H
                add     eax,ecx
                pop     ebx
                ret     
_BrVector2Length:
                push    ebx
                mov     eax,dword ptr (+8H)[esp]
                mov     edx,dword ptr (+4H)[eax]
                push    edx
                mov     ebx,dword ptr [eax]
                push    ebx
                call    near ptr _BrFixedLength2
                add     esp,00000008H
                pop     ebx
                ret     
                mov     eax,eax
_BrVector2LengthSquared:
                push    ebx
                mov     eax,dword ptr (+8H)[esp]
                mov     ebx,dword ptr (+4H)[eax]
                mov     eax,dword ptr [eax]
                imul    eax
                xchg    eax,ebx
                mov     ecx,edx
                imul    eax
                add     eax,ebx
                adc     edx,ecx
                shrd    eax,edx,10H
                pop     ebx
                ret     
                nop     
_BrVector3Copy: push    ebx
                mov     edx,dword ptr (+8H)[esp]
                mov     ebx,dword ptr (+0cH)[esp]
                mov     eax,dword ptr [ebx]
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+4H)[ebx]
                mov     dword ptr (+4H)[edx],eax
                mov     eax,dword ptr (+8H)[ebx]
                mov     dword ptr (+8H)[edx],eax
                pop     ebx
                ret     
                nop     
_BrVector3Set:  mov     eax,dword ptr (+4H)[esp]
                mov     edx,dword ptr (+8H)[esp]
                mov     dword ptr [eax],edx
                mov     edx,dword ptr (+0cH)[esp]
                mov     dword ptr (+4H)[eax],edx
                mov     edx,dword ptr (+10H)[esp]
                mov     dword ptr (+8H)[eax],edx
                ret     
                lea     eax,(+0H)[eax]
_BrVector3SetInt:
                mov     eax,dword ptr (+4H)[esp]
                mov     edx,dword ptr (+8H)[esp]
                shl     edx,10H
                mov     dword ptr [eax],edx
                mov     edx,dword ptr (+0cH)[esp]
                shl     edx,10H
                mov     dword ptr (+4H)[eax],edx
                mov     edx,dword ptr (+10H)[esp]
                shl     edx,10H
                mov     dword ptr (+8H)[eax],edx
                ret     
                mov     eax,eax
_BrVector3SetFloat:
                mov     eax,dword ptr (+4H)[esp]
                fld     dword ptr L6
                fld     dword ptr (+8H)[esp]
                fmul    st,st(1)
                call    near ptr __CHP
                fistp   dword ptr [eax]
                fld     dword ptr (+0cH)[esp]
                fmul    st,st(1)
                call    near ptr __CHP
                fistp   dword ptr (+4H)[eax]
                fmul    dword ptr (+10H)[esp]
                call    near ptr __CHP
                fistp   dword ptr (+8H)[eax]
                ret     
                mov     eax,eax
_BrVector3Negate:
                push    ebx
                push    esi
                push    edi
                mov     edx,dword ptr (+10H)[esp]
                mov     ebx,dword ptr (+14H)[esp]
                mov     eax,dword ptr [ebx]
                mov     ecx,eax
                neg     ecx
                mov     dword ptr [edx],ecx
                mov     eax,dword ptr (+4H)[ebx]
                mov     esi,eax
                neg     esi
                mov     dword ptr (+4H)[edx],esi
                mov     eax,dword ptr (+8H)[ebx]
                mov     edi,eax
                neg     edi
                mov     dword ptr (+8H)[edx],edi
                pop     edi
                pop     esi
                pop     ebx
                ret     
                nop     
_BrVector3Add:  push    ebx
                push    esi
                push    edi
                push    ebp
                mov     edx,dword ptr (+14H)[esp]
                mov     ecx,dword ptr (+18H)[esp]
                mov     ebx,dword ptr (+1cH)[esp]
                mov     eax,dword ptr [ecx]
                mov     esi,dword ptr [ebx]
                add     eax,esi
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+4H)[ecx]
                mov     edi,dword ptr (+4H)[ebx]
                add     eax,edi
                mov     dword ptr (+4H)[edx],eax
                mov     eax,dword ptr (+8H)[ecx]
                mov     ebp,dword ptr (+8H)[ebx]
                add     eax,ebp
                mov     dword ptr (+8H)[edx],eax
                pop     ebp
                pop     edi
                pop     esi
                pop     ebx
                ret     
                nop     
_BrVector3Accumulate:
                push    ebx
                push    esi
                push    edi
                mov     eax,dword ptr (+10H)[esp]
                mov     ebx,dword ptr (+14H)[esp]
                mov     edx,dword ptr [ebx]
                add     dword ptr [eax],edx
                mov     esi,dword ptr (+4H)[eax]
                mov     edx,dword ptr (+4H)[ebx]
                add     esi,edx
                mov     dword ptr (+4H)[eax],esi
                mov     edi,dword ptr (+8H)[eax]
                mov     edx,dword ptr (+8H)[ebx]
                add     edi,edx
                mov     dword ptr (+8H)[eax],edi
                pop     edi
                pop     esi
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
_BrVector3Sub:  push    ebx
                push    esi
                push    edi
                push    ebp
                mov     edx,dword ptr (+14H)[esp]
                mov     ecx,dword ptr (+18H)[esp]
                mov     ebx,dword ptr (+1cH)[esp]
                mov     eax,dword ptr [ecx]
                mov     esi,dword ptr [ebx]
                sub     eax,esi
                mov     dword ptr [edx],eax
                mov     eax,dword ptr (+4H)[ecx]
                mov     edi,dword ptr (+4H)[ebx]
                sub     eax,edi
                mov     dword ptr (+4H)[edx],eax
                mov     eax,dword ptr (+8H)[ecx]
                mov     ebp,dword ptr (+8H)[ebx]
                sub     eax,ebp
                mov     dword ptr (+8H)[edx],eax
                pop     ebp
                pop     edi
                pop     esi
                pop     ebx
                ret     
                nop     
_BrVector3Scale: push    ebx
                push    esi
                push    edi
                mov     esi,dword ptr (+10H)[esp]
                mov     edi,dword ptr (+14H)[esp]
                mov     ecx,dword ptr (+18H)[esp]
                mov     ebx,ecx
                mov     eax,dword ptr [edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr [esi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[esi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+8H)[edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+8H)[esi],eax
                pop     edi
                pop     esi
                pop     ebx
                ret     
                nop     
_BrVector3InvScale:
                push    ebx
                push    esi
                push    edi
                mov     esi,dword ptr (+10H)[esp]
                mov     edi,dword ptr (+14H)[esp]
                mov     ecx,dword ptr (+18H)[esp]
                mov     ebx,ecx
                mov     edx,dword ptr [edi]
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     dword ptr [esi],eax
                mov     ebx,ecx
                mov     edx,dword ptr (+4H)[edi]
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     dword ptr (+4H)[esi],eax
                mov     ebx,ecx
                mov     edx,dword ptr (+8H)[edi]
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     dword ptr (+8H)[esi],eax
                pop     edi
                pop     esi
                pop     ebx
                ret     
                nop     
_BrVector3Dot:  push    ebx
                push    esi
                push    edi
                mov     eax,dword ptr (+10H)[esp]
                mov     edx,dword ptr (+14H)[esp]
                mov     edi,dword ptr (+8H)[edx]
                mov     esi,dword ptr (+8H)[eax]
                mov     ebx,dword ptr (+4H)[edx]
                mov     ecx,dword ptr (+4H)[eax]
                mov     edx,dword ptr [edx]
                mov     eax,dword ptr [eax]
                imul    edx
                shrd    eax,edx,10H
                xchg    eax,ecx
                imul    ebx
                shrd    eax,edx,10H
                add     ecx,eax
                mov     eax,esi
                imul    edi
                shrd    eax,edx,10H
                add     eax,ecx
                pop     edi
                pop     esi
                pop     ebx
                ret     
_BrVector3Cross: push    ebx
                push    esi
                push    edi
                push    ebp
                mov     ebp,dword ptr (+14H)[esp]
                mov     esi,dword ptr (+18H)[esp]
                mov     ecx,dword ptr (+1cH)[esp]
                mov     ebx,dword ptr (+8H)[ecx]
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     edi,eax
                mov     ebx,dword ptr (+4H)[ecx]
                mov     eax,dword ptr (+8H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                sub     edi,eax
                mov     dword ptr (+0H)[ebp],edi
                mov     ebx,dword ptr [ecx]
                mov     eax,dword ptr (+8H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     edi,eax
                mov     ebx,dword ptr (+8H)[ecx]
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                sub     edi,eax
                mov     dword ptr (+4H)[ebp],edi
                mov     ebx,dword ptr (+4H)[ecx]
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     edi,eax
                mov     ebx,dword ptr [ecx]
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                sub     edi,eax
                mov     dword ptr (+8H)[ebp],edi
                pop     ebp
                pop     edi
                pop     esi
                pop     ebx
                ret     
                mov     eax,eax
_BrVector3Length:
                push    ebx
                mov     eax,dword ptr (+8H)[esp]
                mov     edx,dword ptr (+8H)[eax]
                push    edx
                mov     ebx,dword ptr (+4H)[eax]
                push    ebx
                mov     ecx,dword ptr [eax]
                push    ecx
                call    near ptr _BrFixedLength3
                add     esp,0000000cH
                pop     ebx
                ret     
                mov     eax,eax
_BrVector3LengthSquared:
                push    ebx
                push    esi
                mov     eax,dword ptr (+0cH)[esp]
                mov     esi,dword ptr (+8H)[eax]
                mov     ebx,dword ptr (+4H)[eax]
                mov     eax,dword ptr [eax]
                imul    eax
                xchg    eax,ebx
                mov     ecx,edx
                imul    eax
                add     ebx,eax
                adc     ecx,edx
                mov     eax,esi
                imul    eax
                add     eax,ebx
                adc     edx,ecx
                shrd    eax,edx,10H
                pop     esi
                pop     ebx
                ret     
_BrVector3Normalise:
                push    ebx
                push    esi
                push    edi
                mov     edi,dword ptr (+10H)[esp]
                mov     esi,dword ptr (+14H)[esp]
                mov     edx,dword ptr (+8H)[esi]
                push    edx
                mov     ebx,dword ptr (+4H)[esi]
                push    ebx
                mov     ecx,dword ptr [esi]
                push    ecx
                call    near ptr _BrFixedLength3
                add     esp,0000000cH
                cmp     eax,00000002H
                jle     short L1
                mov     edx,00010000H
                mov     ebx,eax
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     ecx,eax
                mov     ebx,ecx
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr [edi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[edi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+8H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+8H)[edi],eax
                pop     edi
                pop     esi
                pop     ebx
                ret     
L1:             mov     dword ptr [edi],00010000H
                mov     dword ptr (+4H)[edi],00000000H
                mov     dword ptr (+8H)[edi],00000000H
                pop     edi
                pop     esi
                pop     ebx
                ret     
                mov     eax,eax
_BrVector3NormaliseLP:
                push    ebx
                push    esi
                push    edi
                mov     edi,dword ptr (+10H)[esp]
                mov     esi,dword ptr (+14H)[esp]
                mov     edx,dword ptr (+8H)[esi]
                push    edx
                mov     ebx,dword ptr (+4H)[esi]
                push    ebx
                mov     ecx,dword ptr [esi]
                push    ecx
                call    near ptr _BrFixedRLength3
                mov     ecx,eax
                add     esp,0000000cH
                test    eax,eax
                je      short L2
                mov     ebx,ecx
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr [edi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[edi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+8H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+8H)[edi],eax
L2:             pop     edi
                pop     esi
                pop     ebx
                ret     
_BrVector4Dot:  push    ebx
                push    esi
                push    edi
                push    ebp
                mov     ebp,dword ptr (+14H)[esp]
                mov     edi,dword ptr (+18H)[esp]
                mov     ebx,dword ptr (+4H)[edi]
                mov     ecx,dword ptr (+4H)[ebp]
                mov     edx,dword ptr [edi]
                mov     eax,dword ptr (+0H)[ebp]
                imul    edx
                shrd    eax,edx,10H
                xchg    eax,ecx
                imul    ebx
                shrd    eax,edx,10H
                add     eax,ecx
                mov     esi,eax
                mov     ebx,dword ptr (+0cH)[edi]
                mov     ecx,dword ptr (+0cH)[ebp]
                mov     edx,dword ptr (+8H)[edi]
                mov     eax,dword ptr (+8H)[ebp]
                imul    edx
                shrd    eax,edx,10H
                xchg    eax,ecx
                imul    ebx
                shrd    eax,edx,10H
                add     eax,ecx
                add     eax,esi
                pop     ebp
                pop     edi
                pop     esi
                pop     ebx
                ret     
                mov     eax,eax
_BrVector4Copy: push    ebx
                mov     edx,dword ptr (+8H)[esp]
                mov     eax,dword ptr (+0cH)[esp]
                mov     ebx,dword ptr [eax]
                mov     dword ptr [edx],ebx
                mov     ebx,dword ptr (+4H)[eax]
                mov     dword ptr (+4H)[edx],ebx
                mov     ebx,dword ptr (+8H)[eax]
                mov     dword ptr (+8H)[edx],ebx
                mov     eax,dword ptr (+0cH)[eax]
                mov     dword ptr (+0cH)[edx],eax
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
BrFVector2Dot_: push    ebx
                push    ecx
                mov     ebx,dword ptr (+4H)[edx]
                mov     cx,word ptr (+2H)[eax]
                mov     edx,dword ptr [edx]
                mov     ax,word ptr [eax]
                cwde    
                imul    edx
                shrd    eax,edx,0fH
                xchg    eax,ecx
                cwde    
                imul    ebx
                shrd    eax,edx,0fH
                add     eax,ecx
                pop     ecx
                pop     ebx
                ret     
                mov     eax,eax
BrFVector3Copy_: push    ebx
                push    ecx
                mov     ecx,eax
                mov     ebx,edx
                mov     eax,dword ptr [edx]
                mov     edx,eax
                sar     edx,1fH
                sub     eax,edx
                sar     eax,1
                mov     word ptr [ecx],ax
                mov     eax,dword ptr (+4H)[ebx]
                mov     edx,eax
                sar     edx,1fH
                sub     eax,edx
                sar     eax,1
                mov     word ptr (+2H)[ecx],ax
                mov     eax,dword ptr (+8H)[ebx]
                mov     edx,eax
                sar     edx,1fH
                sub     eax,edx
                sar     eax,1
                mov     word ptr (+4H)[ecx],ax
                pop     ecx
                pop     ebx
                ret     
                nop     
BrVector3ScaleF_:
                push    ecx
                push    esi
                push    edi
                mov     esi,eax
                mov     edi,edx
                mov     ecx,ebx
                movsx   eax,word ptr [edx]
                add     eax,eax
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr [esi],eax
                mov     eax,dword ptr [edi]
                sar     eax,10H
                mov     ebx,ecx
                add     eax,eax
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[esi],eax
                mov     eax,dword ptr (+2H)[edi]
                sar     eax,10H
                mov     ebx,ecx
                add     eax,eax
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+8H)[esi],eax
                pop     edi
                pop     esi
                pop     ecx
                ret     
                nop     
BrFVector3Dot_: push    ebx
                push    ecx
                push    esi
                push    edi
                mov     edi,dword ptr (+8H)[edx]
                mov     si,word ptr (+4H)[eax]
                mov     ebx,dword ptr (+4H)[edx]
                mov     cx,word ptr (+2H)[eax]
                mov     edx,dword ptr [edx]
                mov     ax,word ptr [eax]
                cwde    
                imul    edx
                shrd    eax,edx,0fH
                xchg    eax,ecx
                cwde    
                imul    ebx
                shrd    eax,edx,0fH
                add     ecx,eax
                mov     eax,esi
                cwde    
                imul    edi
                shrd    eax,edx,0fH
                add     eax,ecx
                pop     edi
                pop     esi
                pop     ecx
                pop     ebx
                ret     
BrFVector3Normalise_:
                push    ebx
                push    ecx
                push    esi
                push    edi
                mov     edi,eax
                mov     esi,edx
                mov     edx,dword ptr (+8H)[edx]
                push    edx
                mov     ebx,dword ptr (+4H)[esi]
                push    ebx
                mov     ecx,dword ptr [esi]
                push    ecx
                call    near ptr _BrFixedLength3
                add     esp,0000000cH
                test    eax,eax
                je      short L3
                mov     edx,00007fffH
                mov     ebx,eax
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     ecx,eax
                mov     ebx,ecx
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr [edi],ax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr (+2H)[edi],ax
                mov     ebx,ecx
                mov     eax,dword ptr (+8H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr (+4H)[edi],ax
                pop     edi
                pop     esi
                pop     ecx
                pop     ebx
                ret     
L3:             mov     word ptr [edi],7fffH
                mov     word ptr (+2H)[edi],0000H
                mov     word ptr (+4H)[edi],0000H
                pop     edi
                pop     esi
                pop     ecx
                pop     ebx
                ret     
BrFVector3NormaliseLP_:
                push    ebx
                push    ecx
                push    esi
                push    edi
                mov     esi,eax
                mov     edi,edx
                mov     eax,dword ptr (+2H)[eax]
                sar     eax,10H
                push    eax
                mov     eax,dword ptr [esi]
                sar     eax,10H
                push    eax
                movsx   eax,word ptr [esi]
                push    eax
                call    near ptr _BrFixedRLength3
                mov     edx,eax
                sar     edx,1fH
                sub     eax,edx
                sar     eax,1
                movsx   ecx,ax
                add     esp,0000000cH
                mov     ebx,ecx
                mov     eax,dword ptr [edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr [esi],ax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr (+2H)[esi],ax
                mov     ebx,ecx
                mov     eax,dword ptr (+8H)[edi]
                imul    ebx
                shrd    eax,edx,10H
                mov     word ptr (+4H)[esi],ax
                pop     edi
                pop     esi
                pop     ecx
                pop     ebx
                ret     
                lea     eax,(+0H)[eax]
_BrVector2Normalise:
                push    ebx
                push    esi
                push    edi
                mov     edi,dword ptr (+10H)[esp]
                mov     esi,dword ptr (+14H)[esp]
                mov     edx,dword ptr (+4H)[esi]
                push    edx
                mov     ebx,dword ptr [esi]
                push    ebx
                call    near ptr _BrFixedLength2
                add     esp,00000008H
                cmp     eax,00000002H
                jle     short L4
                mov     edx,00010000H
                mov     ebx,eax
                mov     eax,edx
                shl     eax,10H
                sar     edx,10H
                idiv    ebx
                mov     ecx,eax
                mov     ebx,ecx
                mov     eax,dword ptr [esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr [edi],eax
                mov     ebx,ecx
                mov     eax,dword ptr (+4H)[esi]
                imul    ebx
                shrd    eax,edx,10H
                mov     dword ptr (+4H)[edi],eax
                pop     edi
                pop     esi
                pop     ebx
                ret     
L4:             mov     dword ptr [edi],00010000H
                mov     dword ptr (+4H)[edi],00000000H
                pop     edi
                pop     esi
                pop     ebx
                ret     
_TEXT           ENDS

CONST           SEGMENT DWORD PUBLIC USE32 'DATA'
L5              DB      00H,00H,80H,47H
L6              DB      00H,00H,80H,47H
CONST           ENDS

CONST2          SEGMENT DWORD PUBLIC USE32 'DATA'
CONST2          ENDS

_DATA           SEGMENT DWORD PUBLIC USE32 'DATA'
_rscid          DB      24H,49H,64H,3aH,20H,76H,65H,63H
                DB      74H,6fH,72H,2eH,63H,20H,31H,2eH
                DB      31H,30H,20H,31H,39H,39H,35H,2fH
                DB      30H,38H,2fH,33H,31H,20H,31H,36H
                DB      3aH,32H,39H,3aH,34H,39H,20H,73H
                DB      61H,6dH,20H,45H,78H,70H,20H,24H
                DB      00H
_DATA           ENDS

_BSS            SEGMENT DWORD PUBLIC USE32 'BSS'
_BSS            ENDS

                END
