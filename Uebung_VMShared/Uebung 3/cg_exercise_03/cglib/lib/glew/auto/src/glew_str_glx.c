    }
    ret = (len == 0);
  }
  return ret;
}

#elif !defined(__ANDROID__) && !defined(__native_client__) && !defined(__HAIKU__) && !defined(__APPLE__) || defined(GLEW_APPLE_GLX)

#if defined(GLEW_MX)
GLboolean glxewContextIsSupported (const GLXEWContext* ctx, const char* name)
#else
GLboolean glxewIsSupported (const char* name)
#endif
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if(_glewStrSame1(&pos, &len, (const GLubyte*)"GLX_", 4))
    {
// CG_REVISION 30dc06794809ebd993cc23fa03a37b35c2d244d5
