#include "backend.hpp"

struct
{
    GLuint program;
    GLint tex_loc;
    GLuint quad;
    GLuint data_image;
} g;

#define GL_ERRCHK(glFn) \
do { \
glFn; \
GLenum err = glGetError(); \
const GLubyte* errmsg = gluErrorString(err); \
if (err != GL_NO_ERROR)    \
    printf("ERROR: 0x%x (%s)\n%s : %d\n", err, errmsg, __FILE__, __LINE__); \
} while (0)
#define GLSL(version, shader)  "#version " #version "\n" #shader
void prepareProgram()
{
    const char* vsource = GLSL(330,
        in vec2 apos;
        out vec2 uv;

        void main()
        {
            gl_Position = vec4(apos, 0.0, 1.0);
            uv = apos * 0.5 + vec2(0.5,0.5);
            uv.y = 1.0-uv.y;
        }
    );
    const char* fsource = GLSL(330,
        in vec2 uv;
        out vec4 fragcolor;

        uniform sampler2D tex;

        void main()
        {
            fragcolor = texture(tex, uv);
            //fragcolor = vec4(uv, 0.0, 1.0);
        }
    );
    // VERTEX SHADER
    GLuint vshdr = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshdr, 1, &vsource, NULL);
    glCompileShader(vshdr);
    // FRAGMENT SHADER
    GLuint fshdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshdr, 1, &fsource, NULL);
    glCompileShader(fshdr);
    // PROGRAM
    g.program = glCreateProgram();
    glAttachShader(g.program, vshdr);
    glAttachShader(g.program, fshdr);
    glLinkProgram(g.program);

    g.tex_loc = glGetUniformLocation(g.program, "tex");

    glDeleteShader(vshdr);
    glDeleteShader(fshdr);
}

void prepareFSQ()
{
    GLfloat verts[12] = { -1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1, 1 };
    glGenBuffers(1, &g.quad);
    glBindBuffer(GL_ARRAY_BUFFER, g.quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawFSQ()
{
    glBindBuffer(GL_ARRAY_BUFFER, g.quad);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "mt.h"

void mainApp()
{
    // setup
    prepareFSQ();
    prepareProgram();
    glGenTextures(1, &g.data_image);
    glBindTexture(GL_TEXTURE_2D, g.data_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL_ERRCHK();
    // l1-l2-l3
    GL_ERRCHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 320 * 3, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0));
    GL_ERRCHK(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 320, 256, GL_LUMINANCE, GL_UNSIGNED_BYTE, data));
    GL_ERRCHK(glTexSubImage2D(GL_TEXTURE_2D, 0, 320, 0, 320, 256, GL_LUMINANCE, GL_UNSIGNED_BYTE, data + 256 * 320));
    GL_ERRCHK(glTexSubImage2D(GL_TEXTURE_2D, 0, 640, 0, 320, 256, GL_LUMINANCE, GL_UNSIGNED_BYTE, data + 256 * 320 * 2));

    // render
    glUseProgram(g.program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g.data_image);
    glUniform1i(g.tex_loc, 0);
    drawFSQ();
    SwapBuffersBackend();
}