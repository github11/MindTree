/*frg.h*/

#ifndef  FRG_INC
#define  FRG_INC

class Project;
class VNSpace;
class DNSpace;
class frg_Shader_Author; 

namespace FRG
{
	extern Project *CurrentProject;
    extern VNSpace *Space;
	extern DNSpace *SpaceDataInFocus;
    extern frg_Shader_Author *Author;
};
#endif   /* ----- #ifndef FRG_INC  ----- */
