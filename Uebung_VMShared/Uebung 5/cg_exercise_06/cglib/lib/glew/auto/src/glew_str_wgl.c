    }
    ret = (len == 0);
  }
  return ret;
}

#if defined(_WIN32)

#if defined(GLEW_MX)
GLboolean GLEWAPIENTRY wglewContextIsSupported (const WGLEWContext* ctx, const char* name)
#else
GLboolean GLEWAPIENTRY wglewIsSupported (const char* name)
#endif
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"WGL_", 4))
    {
// CG_REVISION 5864f116abf20cc32f7d7b04704e99acebcf643c
