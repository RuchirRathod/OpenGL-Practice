#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

using namespace std;

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        cout << "[OpenGL Error] ";
        switch (error) {
        case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM : An unacceptable value is specified for an enumerated argument.";
            break;
        case GL_INVALID_VALUE:
            cout << "GL_INVALID_OPERATION : A numeric argument is out of range.";
            break;
        case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION : The specified operation is not allowed in the current state.";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION : The framebuffer object is not complete.";
            break;
        case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY : There is not enough memory left to execute the command.";
            break;
        case GL_STACK_UNDERFLOW:
            cout << "GL_STACK_UNDERFLOW : An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            break;
        case GL_STACK_OVERFLOW:
            cout << "GL_STACK_OVERFLOW : An attempt has been made to perform an operation that would cause an internal stack to overflow.";
            break;
        default:
            cout << "Unrecognized error" << error;
        }
        cout << endl;
        return false;
    }
    return true;
}

struct ShaderProgramSource
{
    string VertexSource;
    string FragmentSource;
};

static ShaderProgramSource ParseShader(const string& filepath) 
{
    ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    string line;
    stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {        
        if (line.find("#shader") != string::npos)
        {            
            if (line.find("vertex") != string::npos)
            {
                type = ShaderType::VERTEX;                
            }
            else if (line.find("fragment") != string::npos)
            {
                type = ShaderType::FRAGMENT;                
            }            
        }
        else
        {                 
            ss[(int)type] << line << "\n";
        }
    }    
    return {ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader." << endl;
        cout << message << endl;
        GLCall(glDeleteShader(id));
        return 0;
    }
    
    return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        cout << "Error!" << endl;
    }

    cout << glGetString(GL_VERSION) << endl;

    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));
    
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0));

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}