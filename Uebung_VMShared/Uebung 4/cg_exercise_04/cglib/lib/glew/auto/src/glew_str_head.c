#ifdef GLEW_MX
GLboolean GLEWAPIENTRY glewContextIsSupported (const GLEWContext* ctx, const char* name)
#else
GLboolean GLEWAPIENTRY glewIsSupported (const char* name)
#endif
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"GL_", 3))
    {
// CG_REVISION 853babea119d24f6c9aca86013fd5e6a3fb22330
