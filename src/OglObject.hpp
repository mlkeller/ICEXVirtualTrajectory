#pragma once
#ifndef KJY_OPENGL_OBJECT_H_
#define KJY_OPENGL_OBJECT_H_
#include <glad/glad.h>
#include <memory>

struct OglName{
   OglName(void (*destroyer)(OglName& self)) : destroyer(destroyer){oglID = 0;}
   ~OglName() {if(destroyer != nullptr) destroyer(*this);}
   GLuint* operator&(){return(&oglID);}
   GLuint oglID;
 protected:
   void (*destroyer)(OglName& self) = nullptr;
};

typedef std::shared_ptr<OglName> PersistentOglName;

struct OglObject{
   GLuint ID() const {return(coreobj->oglID); }
   GLuint* getIdPtr(){return(&coreobj->oglID);}
   
   PersistentOglName coreobj = nullptr;
};

#endif