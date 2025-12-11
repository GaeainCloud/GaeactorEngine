#if 1
#include "./src/uiwidget/widgetmanager.h"
#include <QApplication>
#include <QTranslator>
#ifdef WIN32
#include "crashhelper.h"
#include <windows.h>
#endif
#include <QQmlApplicationEngine>
#include "easy/profiler.h"

#include "settingsconfig.h"
#include "loghelper.h"
#include "runningmodeconfig.h"
int main(int argc, char* argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QApplication a(argc, argv);

    QString groupname ="default";
    switch (argc)
    {
    case 2:
    {
        groupname = a.arguments().at(1);
    }
    break;
    }

    SettingsConfig::getInstance().setGroupname(groupname);
    gaeactorcomm::GaeactorComm::getInstance().init(SettingsConfig::getInstance().getCommserviceName().c_str());

	runningmode::RunningModeConfig::getInstance().load();
	log_service::LogHelper::instance("lavaic-desktop")->initLog("lavaic-desktop");
        profiler::startListen(29363);
#if 1
#ifdef Q_OS_WIN32
	// dump
	gaeactor_dump::CrashDumpHelper::setDump("lavic_desktop");
#elif defined(Q_OS_LINUX)
	///TODO:Linux
#endif

#ifdef WIN32
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif
	QCoreApplication::setOrganizationName("Some organization");
	//    eqnx_dh::LogHelper::instance()->initLog();
	QStringList qmFilePathList;
	qmFilePathList.append("res/qm/gaeactor_display_cn.qm");

	for (auto qmFilePath : qmFilePathList)
	{
		auto translator = new QTranslator;
		translator->load(qmFilePath);
		QApplication::installTranslator(translator);
	}

	//    QQmlApplicationEngine engine;
	//    const QUrl url(QStringLiteral("qrc:/qml/globalobject.qml"));

	//    //engine->load(QUrl("qrc:///globalobject.qml"));
	//    engine.load(url);

	WidgetManager w;
	w.run();
	w.resize(1920, 1080);
	w.show();

#else

#include "./src/components/eventdriver/eventdriver.h"

	EventDriver* pEventDriver = new EventDriver();
	tagEventInfo timereventinfo;
	timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_REPEAT_PERIOD;
	timereventinfo.m_eventId = FunctionAssistant::generate_random_positive_uint64();
	timereventinfo.timeout = 2;
	timereventinfo.repeattimes = 0;
	timereventinfo.m_eventtype = E_EVENT_TYPE_ID_UNKOWN;
	timereventinfo.flightevent = nullptr;
	pEventDriver->addevent(timereventinfo);

	pEventDriver->start();
#endif
	return a.exec();
}
#else
#define M_PI                      3.14159265358979323
#if 1
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#if 1
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <vector>
//#include <cmath>
//
//const unsigned int SCR_WIDTH = 800;
//const unsigned int SCR_HEIGHT = 600;
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, true);
//}
//

//
//int main()
//{
//	// Initialize GLFW
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	// Create a window
//	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Transparent Fan", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed to create GLFW window" << std::endl;
//		glfwTerminate();
//		return -1;
//	}
//	glfwMakeContextCurrent(window);
//
//	// Initialize GLEW
//	glewExperimental = GL_TRUE; // Needed for core profile
//	if (glewInit() != GLEW_OK)
//	{
//		std::cout << "Failed to initialize GLEW" << std::endl;
//		return -1;
//	}
//
//	// Define the viewport dimensions
//	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//	// Compile shaders
//	// Vertex shader
//	const char* vertexShaderSource1 = "#version 330 core\n"
//		"layout (location = 0) in vec2 position;\n"
//		"layout(location = 1) in vec4 VertexColor; \n"
//		"out vec4 Color;\n"
//		"uniform float rotationAngle;\n"
//		"void main()\n"
//		"{\n"
//		"Color =  VertexColor;\n"
//		//"mat2 rotationMatrix = mat2(cos(rotationAngle), -sin(rotationAngle),\n"
//		//"	sin(rotationAngle), cos(rotationAngle));\n"
//				"mat2 rotationMatrix = mat2(cos(rotationAngle), sin(rotationAngle),\n"
//		"	-sin(rotationAngle), cos(rotationAngle));\n"
//
//		"vec2 rotatedPosition = rotationMatrix * position;\n"
//	"gl_Position = vec4(rotatedPosition,0.0, 1.0);\n"
//		"}\0";
//	unsigned int vertexShader;
//	vertexShader = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(vertexShader, 1, &vertexShaderSource1, NULL);
//	glCompileShader(vertexShader);
//
//	// Check for vertex shader compile errors
//	int success;
//	char infoLog[512];
//	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//
//	// Fragment shader
//	const char* fragmentShaderSource1 = "#version 330 core\n"
//		"out vec4 FragColor;\n"
//		"in vec4 Color;\n"
//		"void main()\n"
//		"{\n"
//		"    FragColor =Color;\n"
//		"}\n\0";
//	unsigned int fragmentShader;
//	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(fragmentShader, 1, &fragmentShaderSource1, NULL);
//	glCompileShader(fragmentShader);
//
//	// Check for fragment shader compile errors
//	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//	}
//
//
//	// Link shaders
//	unsigned int shaderProgram;
//	shaderProgram = glCreateProgram();
//	glAttachShader(shaderProgram, vertexShader);
//	glAttachShader(shaderProgram, fragmentShader);
//	glLinkProgram(shaderProgram);
//
//	// Check for shader linking errors
//	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//	if (!success)
//	{
//		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//	}
//
//	glDeleteShader(vertexShader);
//	glDeleteShader(fragmentShader);
//
//	const int numSegments = 360;
//
//
//	std::vector<glm::dvec2> m_pVerticesVector;
//	m_pVerticesVector.resize(numSegments + 1);
//	float vertices[(numSegments + 2) * 2];  // (numSegments + 2) points, each with 2 coordinates (x, y)
//
//	vertices[0] = 0.0f;  // Center point
//	vertices[1] = 0.0f;
//	m_pVerticesVector[0] = glm::dvec2(0.0, 0.0);
//	double anglesstart = 0.0f * (M_PI / 180.0f);
//
//	double angless = 45 * (M_PI / 180.0f);
//	float angleStep = angless / numSegments;
//	for (int i = 0; i < numSegments; ++i)
//	{
//		float angle = anglesstart + angleStep * i;
//		//vertices[2 * (i + 1)] = cos(angle);  // x coordinate
//		//vertices[2 * (i + 1) + 1] = sin(angle);  // y coordinate
//
//		m_pVerticesVector[i+1]= glm::dvec2(cos(angle), sin(angle));
//	}
//	std::vector<glm::vec4> m_pclolorsVector;
//	m_pclolorsVector.resize(numSegments);
//
//	for (int i = 0; i < numSegments; i++)
//	{
//		m_pclolorsVector[i] = glm::vec4(0.0, 1.0, 0.0, (float)i/ (float)numSegments);
//	}
//
//
//	unsigned int VBO, VAO,colorvbo;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	glGenBuffers(1, &colorvbo);
//
//	glBindVertexArray(VAO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::dvec2) * numSegments, m_pVerticesVector.data(), GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, sizeof(glm::dvec2) / sizeof(double), GL_FLOAT, GL_FALSE, sizeof(glm::dvec2) / sizeof(double) * sizeof(double), (void*)0);
//	glEnableVertexAttribArray(0);
//
//	/* 颜色VBO */
//	glBindBuffer(GL_ARRAY_BUFFER, colorvbo);// 绑定顶点数据缓存空间
//	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numSegments, m_pclolorsVector.data(), GL_STATIC_DRAW);
//	// 解析VBO中的数据
//	glVertexAttribPointer(1, sizeof(glm::vec4) / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(glm::vec4) / sizeof(GLfloat) * sizeof(GLfloat), (void*)0);
//	// 启用顶点属性==>即着色器中(location=1)的顶点属性
//	glEnableVertexAttribArray(1);
//
//	// Unbind VAO
//	glBindVertexArray(0);
//	float alpha = 0.0; // Example of dynamic alpha change
//	float rotationSpeed = 5.0;
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	// Rendering loop
//	while (!glfwWindowShouldClose(window))
//	{
//		// Input
//		processInput(window);
//
//		// Rendering commands here
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT);
//
//		// Use shader program
//		glUseProgram(shaderProgram);
//
//		// Set color and alpha values
//		float time = glfwGetTime();
//
//		float rotationAngle = glfwGetTime() * rotationSpeed; // rotationSpeed can be adjusted
//		int rotationLoc = glGetUniformLocation(shaderProgram, "rotationAngle");
//		glUniform1f(rotationLoc, rotationAngle);
//		// Draw fan
//		glBindVertexArray(VAO);
//		glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments+2);
//		glBindVertexArray(0);
//
//		// Check and call events and swap the buffers
//		glfwPollEvents();
//		glfwSwapBuffers(window);
//	}
//
//	// Cleanup
//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//	glDeleteProgram(shaderProgram);
//
//	// Terminate GLFW
//	glfwTerminate();
//	return 0;
//}

//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <vector>
//#include <cmath>
//#include <glm/vec4.hpp>
//
//const unsigned int SCR_WIDTH = 800;
//const unsigned int SCR_HEIGHT = 600;
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}
//
//void processInput(GLFWwindow* window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//}
//
//struct Vertex {
//    glm::vec2 position;
//    glm::vec4 color;
//};
//
//int main()
//{
//    // Initialize GLFW
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//    // Create a window
//    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Transparent Fan", NULL, NULL);
//    if (window == NULL)
//    {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//
//    // Initialize GLEW
//    glewExperimental = GL_TRUE; // Needed for core profile
//    if (glewInit() != GLEW_OK)
//    {
//        std::cout << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    // Define the viewport dimensions
//    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    // Compile shaders
//    // Vertex shader
//    const char* vertexShaderSource1 = "#version 330 core\n"
//        "layout (location = 0) in vec2 position;\n"
//        "layout(location = 1) in vec4 VertexColor;\n"
//        "out vec4 Color;\n"
//        "uniform float rotationAngle;\n"
//        "void main()\n"
//        "{\n"
//        "    Color = VertexColor;\n"
//        "    mat2 rotationMatrix = mat2(cos(rotationAngle), sin(rotationAngle),\n"
//        "                               -sin(rotationAngle), cos(rotationAngle));\n"
//        "    vec2 rotatedPosition = rotationMatrix * position;\n"
//        "    gl_Position = vec4(rotatedPosition, 0.0, 1.0);\n"
//        "}\0";
//
//    unsigned int vertexShader;
//    vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource1, NULL);
//    glCompileShader(vertexShader);
//
//    // Check for vertex shader compile errors
//    int success;
//    char infoLog[512];
//    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//    }
//
//    // Fragment shader
//    const char* fragmentShaderSource1 = "#version 330 core\n"
//        "out vec4 FragColor;\n"
//        "in vec4 Color;\n"
//        "void main()\n"
//        "{\n"
//        "    FragColor = Color;\n"
//        "}\n\0";
//
//    unsigned int fragmentShader;
//    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource1, NULL);
//    glCompileShader(fragmentShader);
//
//    // Check for fragment shader compile errors
//    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//    if (!success)
//    {
//        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//    }
//
//    // Link shaders
//    unsigned int shaderProgram;
//    shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//    // Check for shader linking errors
//    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//    if (!success)
//    {
//        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//    }
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    // Setup vertices and colors
//    const int numSegments = 360;
//    std::vector<Vertex> vertices;
//    vertices.resize(numSegments + 2);
//
//    vertices[0].position = glm::vec2(0.0f, 0.0f);
//    vertices[0].color = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Center point color
//
//    double angleStart = 0.0f * (M_PI / 180.0f);
//    float angleStep = (30.0f / numSegments) * (M_PI / 180.0f);
//
//    for (int i = 0; i <= numSegments; ++i)
//    {
//        float angle = angleStart + angleStep * i;
//        vertices[i + 1].position = glm::vec2(cos(angle), sin(angle)); // x, y coordinates
//        vertices[i + 1].color = glm::vec4(0.0f, 1.0f, 0.0f, (float)i / (float)numSegments); // Example color based on segment index
//    }
//
//    unsigned int VBO, VAO;
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
//
//    // Position attribute
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
//    glEnableVertexAttribArray(0);
//
//    // Color attribute
//    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
//    glEnableVertexAttribArray(1);
//
//    // Unbind VAO
//    glBindVertexArray(0);
//
//    float rotationSpeed = 1.0f;
//
//    // Enable blending for transparency
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//    // Rendering loop
//    while (!glfwWindowShouldClose(window))
//    {
//        // Input handling
//        processInput(window);
//
//        // Rendering commands
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // Use shader program
//        glUseProgram(shaderProgram);
//
//        // Set rotation angle
//        float rotationAngle = glfwGetTime() * rotationSpeed;
//        glUniform1f(glGetUniformLocation(shaderProgram, "rotationAngle"), rotationAngle);
//
//        // Draw fan
//        glBindVertexArray(VAO);
//        glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 2);
//        glBindVertexArray(0);
//
//        // Swap buffers and poll events
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // Cleanup
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//
//    // Terminate GLFW
//    glfwTerminate();
//    return 0;
//}

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

struct Vertex {
    glm::vec3 position; // Changed to vec3
    glm::vec4 color;
};

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Transparent Fan", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Compile shaders
    // Vertex shader
    const char* vertexShaderSource1 = "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n" // Changed to vec3
        "layout(location = 1) in vec4 VertexColor;\n"
        "out vec4 Color;\n"
        "uniform float rotationAngle;\n"
        "void main()\n"
        "{\n"
        "    Color = VertexColor;\n"
        "    mat2 rotationMatrix = mat2(cos(rotationAngle), sin(rotationAngle),\n"
        "                               -sin(rotationAngle), cos(rotationAngle));\n"
        "    vec2 rotatedPosition = rotationMatrix * position.xy;\n" // Only rotate x and y
        "    gl_Position = vec4(rotatedPosition, 0.0, 1.0);\n" // Keep z as 0
        "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource1, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment shader
    const char* fragmentShaderSource1 = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec4 Color;\n"
        "void main()\n"
        "{\n"
        "    FragColor = Color;\n"
        "}\n\0";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource1, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup vertices and colors
    const int numSegments = 360;
    std::vector<Vertex> vertices;
    vertices.resize(numSegments + 2);

    vertices[0].position = glm::vec3(0.0f, 0.0f, 0.0f); // Center point
    vertices[0].color = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f); // Center point color

    double angleStart = 0.0f * (M_PI / 180.0f);
    float angleStep = (60.0f / numSegments) * (M_PI / 180.0f);

    for (int i = 0; i <= numSegments; ++i)
    {
        float angle = angleStart + angleStep * i;
        vertices[i + 1].position = glm::vec3(cos(angle), sin(angle), 0.0f); // x, y coordinates (z = 0)
        vertices[i + 1].color = glm::vec4(0.0f, 1.0f, 0.0f, (float)i / (float)numSegments); // Example color based on segment index
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);

    float rotationSpeed = 1.0f;

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Rendering loop
    while (!glfwWindowShouldClose(window))
    {
        // Input handling
        processInput(window);

        // Rendering commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use shader program
        glUseProgram(shaderProgram);

        // Set rotation angle
        float rotationAngle = glfwGetTime() * rotationSpeed;
        glUniform1f(glGetUniformLocation(shaderProgram, "rotationAngle"), rotationAngle);

        // Draw fan
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 2);
        glBindVertexArray(0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}


#else

#endif



#endif



#if 0
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

const char* vertexShaderSource1 = R"(
    #version 330 core
    layout (location = 0) in vec2 position;

    void main()
    {
        gl_Position = vec4(position, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource1 = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec2 center;
    uniform float time;
    uniform float scanSpeed;
    uniform float lineWidth;
    uniform vec4 scanColorStart;
    uniform vec4 scanColorEnd;

    void main()
    {
        // 计算当前片段相对于中心的坐标和极坐标角度
        vec2 normCoord = (gl_FragCoord.xy - center) / max(abs(center.x), abs(center.y));
        //float angle = atan(normCoord.y, normCoord.x);
        //float normalizedAngle = (angle < 0.0) ? (angle + 2.0 * 3.14159) : angle;
        //
        //// 计算当前时间对应的扫描角度
        //float currentAngle = mod(time * scanSpeed, 2.0 * 3.14159);
        //
        //// 计算当前片段与扫描角度之间的角度差
        //float angleDifference = abs(normalizedAngle - currentAngle);
        //
        //// 使用 smoothstep 函数来实现雷达扫描效果
        //float alpha = smoothstep(1.0 - lineWidth, 1.0, angleDifference);
        
        // 计算颜色渐变
        float distanceToCenter = length(normCoord);
        vec4 scanColor = mix(scanColorStart, scanColorEnd, smoothstep(0.0, 1.0, distanceToCenter));

        // 最终颜色为扫描线颜色和透明度
//        FragColor = scanColor * alpha;
        FragColor = scanColor ;
    }
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	// 初始化GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	// 创建窗口
	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Radar Scan Example", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 初始化GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// 编译和链接顶点着色器和片段着色器
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource1, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource1, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// 设置扫描参数
	GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
	GLint timeLoc = glGetUniformLocation(shaderProgram, "time");
	GLint scanSpeedLoc = glGetUniformLocation(shaderProgram, "scanSpeed");
	GLint lineWidthLoc = glGetUniformLocation(shaderProgram, "lineWidth");
	GLint scanColorStartLoc = glGetUniformLocation(shaderProgram, "scanColorStart");
	GLint scanColorEndLoc = glGetUniformLocation(shaderProgram, "scanColorEnd");

	// 扫描参数
	float centerX = 400.0f;         // 中心点 x 坐标
	float centerY = 300.0f;         // 中心点 y 坐标
	float radius = 300.0f;          // 扫描半径
	float scanSpeed = 1.0f;         // 扫描速度，可以调整扫描的快慢
	float lineWidth = 0.02f;        // 扫描线宽度
	float scanColorStart[4] = { 0.0f, 1.0f, 0.0f, 1.0f };  // 扫描线起始颜色，绿色
	float scanColorEnd[4] = { 0.0f, 0.0f, 0.0f, 0.0f };    // 扫描线结束颜色，透明

	// 渲染循环
	while (!glfwWindowShouldClose(window))
	{
		// 获取当前时间
		float time = glfwGetTime();

		// 清空颜色缓冲区
		glClear(GL_COLOR_BUFFER_BIT);

		// 使用着色器程序
		glUseProgram(shaderProgram);

		// 设置uniform变量的值
		glUniform2f(centerLoc, centerX, centerY);
		glUniform1f(timeLoc, time);
		glUniform1f(scanSpeedLoc, scanSpeed);
		glUniform1f(lineWidthLoc, lineWidth);
		glUniform4fv(scanColorStartLoc, 1, scanColorStart);
		glUniform4fv(scanColorEndLoc, 1, scanColorEnd);

		// 绘制一个矩形来触发片段着色器
		glBegin(GL_TRIANGLES);
		glVertex2f(-1.0f, -1.0f);
		glVertex2f(3.0f, -1.0f);
		glVertex2f(-1.0f, 3.0f);
		glEnd();

		// 交换缓冲区和检查事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 清理资源
	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}
#endif
//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <iostream>
//#include <cmath>
//
//const char* vertexShaderSource1 = R"(
//    #version 330 core
//    layout (location = 0) in vec2 position;
//
//    void main()
//    {
//        gl_Position = vec4(position, 0.0, 1.0);
//    }
//)";
//
//const char* fragmentShaderSource1 = R"(
//    #version 330 core
//    out vec4 FragColor;
//
//    uniform vec2 center;
//    uniform float time;
//    uniform float scanSpeed;
//    uniform float lineWidth;
//    uniform vec4 scanColor;
//    uniform vec4 gradientColorStart;
//    uniform vec4 gradientColorEnd;
//
//    void main()
//    {
//        // 计算当前片段相对于中心的坐标和极坐标角度
//        vec2 normCoord = (gl_FragCoord.xy - center) / max(abs(center.x), abs(center.y));
//        float angle = atan(normCoord.y, normCoord.x);
//        float normalizedAngle = (angle < 0.0) ? (angle + 2.0 * 3.14159) : angle;
//        
//        // 计算当前时间对应的扫描角度
//        float currentAngle = mod(time * scanSpeed, 2.0 * 3.14159);
//        
//        // 计算当前片段与扫描角度之间的角度差
//        float angleDifference = abs(normalizedAngle - currentAngle);
//        
//        // 使用 smoothstep 函数来实现圆形扫描效果
//        float alpha = smoothstep(1.0 - lineWidth, 1.0, angleDifference);
//        
//        // 计算全方位360度循环扫描效果
//        float distanceToCenter = length(normCoord);
//        float scanRadius = 0.5; // 360度扫描的半径范围
//        float circularAlpha = smoothstep(1.0 - lineWidth, 1.0, distanceToCenter - scanRadius);
//        
//        // 结合两种效果，取较大的透明度值
//        alpha = max(alpha, circularAlpha);
//        
//        // 计算圆锥渐变效果
//        float gradientAngle = atan(normCoord.y, normCoord.x);
//        float gradientLength = length(normCoord);
//        float gradientFactor = (gradientAngle - currentAngle) / (2.0 * 3.14159);
//        vec4 gradientColor = mix(gradientColorStart, gradientColorEnd, gradientFactor);
//        
//        // 最终颜色为圆锥渐变和扫描线混合后的结果
//        FragColor = mix(scanColor * alpha, gradientColor, smoothstep(0.0, 1.0, gradientLength));
//    }
//)";
//
//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//    glViewport(0, 0, width, height);
//}
//
//int main()
//{
//    // 初始化GLFW
//    if (!glfwInit())
//    {
//        std::cerr << "Failed to initialize GLFW" << std::endl;
//        return -1;
//    }
//
//    // 创建窗口
//    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Radar Scan Example", NULL, NULL);
//    if (!window)
//    {
//        std::cerr << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//
//    glfwMakeContextCurrent(window);
//    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//    // 初始化GLEW
//    if (glewInit() != GLEW_OK)
//    {
//        std::cerr << "Failed to initialize GLEW" << std::endl;
//        return -1;
//    }
//
//    // 编译和链接顶点着色器和片段着色器
//    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertexShaderSource1, NULL);
//    glCompileShader(vertexShader);
//
//    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragmentShaderSource1, NULL);
//    glCompileShader(fragmentShader);
//
//    GLuint shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, vertexShader);
//    glAttachShader(shaderProgram, fragmentShader);
//    glLinkProgram(shaderProgram);
//
//    glDeleteShader(vertexShader);
//    glDeleteShader(fragmentShader);
//
//    // 设置扫描参数
//    GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
//    GLint timeLoc = glGetUniformLocation(shaderProgram, "time");
//    GLint scanSpeedLoc = glGetUniformLocation(shaderProgram, "scanSpeed");
//    GLint lineWidthLoc = glGetUniformLocation(shaderProgram, "lineWidth");
//    GLint scanColorLoc = glGetUniformLocation(shaderProgram, "scanColor");
//    GLint gradientColorStartLoc = glGetUniformLocation(shaderProgram, "gradientColorStart");
//    GLint gradientColorEndLoc = glGetUniformLocation(shaderProgram, "gradientColorEnd");
//
//    // 扫描参数
//    float centerX = 400.0f;         // 中心点 x 坐标
//    float centerY = 300.0f;         // 中心点 y 坐标
//    float radius = 300.0f;          // 扫描半径
//    float scanSpeed = 1.0f;         // 扫描速度，可以调整扫描的快慢
//    float lineWidth = 0.02f;        // 扫描线宽度
//    float scanColor[4] = { 0.0f, 1.0f, 0.0f, 1.0f };  // 绿色
//    float gradientColorStart[4] = { 1.0f, 1.0f, 1.0f, 0.0f };  // 渐变起始颜色，透明白色
//    float gradientColorEnd[4] = { 1.0f, 1.0f, 1.0f, 1.0f };    // 渐变结束颜色，不透明白色
//
//    // 渲染循环
//    while (!glfwWindowShouldClose(window))
//    {
//        // 获取当前时间
//        float time = glfwGetTime();
//
//        // 清空颜色缓冲区
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // 使用着色器程序
//        glUseProgram(shaderProgram);
//
//        // 设置uniform变量的值
//        glUniform2f(centerLoc, centerX, centerY);
//        glUniform1f(timeLoc, time);
//        glUniform1f(scanSpeedLoc, scanSpeed);
//        glUniform1f(lineWidthLoc, lineWidth);
//        glUniform4fv(scanColorLoc, 1, scanColor);
//        glUniform4fv(gradientColorStartLoc, 1, gradientColorStart);
//        glUniform4fv(gradientColorEndLoc, 1, gradientColorEnd);
//
//        // 绘制一个矩形来触发片段着色器
//        glBegin(GL_TRIANGLES);
//        glVertex2f(-1.0f, -1.0f);
//        glVertex2f(3.0f, -1.0f);
//        glVertex2f(-1.0f, 3.0f);
//        glEnd();
//
//        // 交换缓冲区和检查事件
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//
//    // 清理资源
//    glDeleteProgram(shaderProgram);
//    glfwTerminate();
//    return 0;
//}






//#include <osgViewer/Viewer>
//#include <osg/Geode>
//#include <osg/Geometry>
//#include <osg/LineWidth>
//#include <osg/ShapeDrawable>
//#include <osgText/Text>
//#include <osg/MatrixTransform>
//
//
//
//const char* vertexShaderSource1 = R"(
//    #version 330 core
//    layout (location = 0) in vec3 position;
//	uniform mat4 transform;
//    void main()
//    {
//        gl_Position = transform * vec4(position, 1.0);
//    }
//)";
//
//const char* vertexShaderSource2 = R"(
//    #version 330 core
//    layout (location = 0) in vec3 position;
//	void main()
//    {
//        gl_Position = vec4(position, 1.0);
//    }
//)";
//
//const char* vertexShaderSource3 = R"(
//	#version 430
//	uniform mat4 osg_ModelViewProjectionMatrix;
//	in vec4 osg_Vertex;
//	in vec3 osg_Normal;
//	out vec3 color;
//	void main()
//	{
//		float theta = dot(osg_Normal, normalize(vec3(0.0, 1.0, 0.0)));
//		color = vec3(0.0, 1.0, 0.0)*theta;
//
//		gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
//	}
//
//)";
//
//
//
//const char* geometryShaderSource1 = R"(
//    #version 330 core
//    layout (points) in;
//    layout (triangle_strip, max_vertices = 4) out;
//
//    void main()
//    {
//        gl_Position = gl_in[0].gl_Position + vec4(-0.001, -0.001, 0.0, 0.0);
//        EmitVertex();
//
//        gl_Position = gl_in[0].gl_Position + vec4(0.001, -0.001, 0.0, 0.0);
//        EmitVertex();
//
//        gl_Position = gl_in[0].gl_Position + vec4(-0.001, 0.001, 0.0, 0.0);
//        EmitVertex();
//
//        gl_Position = gl_in[0].gl_Position + vec4(0.001, 0.001, 0.0, 0.0);
//        EmitVertex();
//
//        EndPrimitive();
//    }
//)";
//
////const char* geometryShaderSource = R"(
////    #version 330 core
////    layout (points) in;
////    layout (points, max_vertices = 4) out;
////
////    void main()
////    {
////        gl_Position = gl_in[0].gl_Position;
////        EmitVertex();
////
////        EndPrimitive();
////    }
////)";
//
//const char* fragmentShaderSource1 = R"(
//    #version 330 core
//    out vec4 FragColor;
//
//    void main()
//    {
//        FragColor = vec4(1.0, 1.0, 0.0, 1.0);
//    }
//)";
//
//
//const char* fragmentShaderSource2 = R"(
//	#version 430
//	in vec3 color;
//	out vec4 FragColor;
//	void main()
//	{
//		FragColor = vec4(color, 1.0);
//	}
//)";
//#include <osgGA/TrackballManipulator>
//
//#include<osgManipulator/TabBoxDragger>
//#include<osgManipulator/Selection>
//#include<osg/MatrixTransform>
//#include<osgGA/GUIEventAdapter>
//#include<osgManipulator/TrackballDragger>
//#include<osgManipulator/CommandManager>
//
//#include <osgManipulator/TranslateAxisDragger>
//
//#include <osgDB/ReadFile>
//
//int main(int argc, char** argv)
//{
//#if 0
//	// 创建一个查看器
//	osgViewer::Viewer viewer;
//
//	// 创建一个根节点
//	osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
//
//	// 创建一个几何节点用于绘制坐标系
//	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
//
//	// 创建X轴线段
//	osg::ref_ptr<osg::Geometry> xAxisGeom = new osg::Geometry;
//	osg::ref_ptr<osg::Vec3Array> xAxisVertices = new osg::Vec3Array;
//	xAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	xAxisVertices->push_back(osg::Vec3(1.0, 0.0, 0.0));
//	xAxisGeom->setVertexArray(xAxisVertices);
//	osg::ref_ptr<osg::Vec4Array> xAxisColors = new osg::Vec4Array;
//	xAxisColors->push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0)); // 红色
//	xAxisGeom->setColorArray(xAxisColors, osg::Array::BIND_OVERALL);
//	xAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	osg::ref_ptr<osg::LineWidth> xAxisLineWidth = new osg::LineWidth(3.0f);
//	xAxisGeom->getOrCreateStateSet()->setAttributeAndModes(xAxisLineWidth, osg::StateAttribute::ON);
//
//
//	// 创建y轴线段
//	osg::ref_ptr<osg::Geometry> yAxisGeom = new osg::Geometry;
//	osg::ref_ptr<osg::Vec3Array> yAxisVertices = new osg::Vec3Array;
//	yAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	yAxisVertices->push_back(osg::Vec3(0.0, 1.0, 0.0));
//	yAxisGeom->setVertexArray(yAxisVertices);
//	osg::ref_ptr<osg::Vec4Array> yAxisColors = new osg::Vec4Array;
//	yAxisColors->push_back(osg::Vec4(0.0, 0.0, 1.0, 1.0)); // 红色
//	yAxisGeom->setColorArray(yAxisColors, osg::Array::BIND_OVERALL);
//	yAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	osg::ref_ptr<osg::LineWidth> yAxisLineWidth = new osg::LineWidth(3.0f);
//	yAxisGeom->getOrCreateStateSet()->setAttributeAndModes(yAxisLineWidth, osg::StateAttribute::ON);
//
//
//
//	// 创建z轴线段
//	osg::ref_ptr<osg::Geometry> zAxisGeom = new osg::Geometry;
//	osg::ref_ptr<osg::Vec3Array> zAxisVertices = new osg::Vec3Array;
//	zAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	zAxisVertices->push_back(osg::Vec3(0.0, 0.0, 1.0));
//	zAxisGeom->setVertexArray(zAxisVertices);
//	osg::ref_ptr<osg::Vec4Array> zAxisColors = new osg::Vec4Array;
//	zAxisColors->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0)); // 红色
//	zAxisGeom->setColorArray(zAxisColors, osg::Array::BIND_OVERALL);
//	zAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	osg::ref_ptr<osg::LineWidth> zAxisLineWidth = new osg::LineWidth(3.0f);
//	zAxisGeom->getOrCreateStateSet()->setAttributeAndModes(zAxisLineWidth, osg::StateAttribute::ON);
//
//
//
//
//	geode->addDrawable(xAxisGeom);
//	geode->addDrawable(yAxisGeom);
//	geode->addDrawable(zAxisGeom);
//
//
//	osg::ref_ptr<osg::Camera> camera = viewer.getCamera();
//
//	//// 设置相机的视图矩阵，将Y轴设为上方向
//	//osg::Matrix viewMatrix = osg::Matrix::identity();
//	//viewMatrix.set(osg::Matrix::lookAt(osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0), osg::Vec3(0, 0, -1)));
//	//camera->setViewMatrix(viewMatrix);
//
//	osg::Matrix worldMatrix = osg::Matrix::identity();
//	worldMatrix.set(osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3(1, 0, 0)));
//	root->setMatrix(worldMatrix);
//
//	// 将几何节点添加到根节点
//	root->addChild(geode);
//
//	// 设置查看器的根节点
//	viewer.setSceneData(root);
//
//	// 运行查看器
//	return viewer.run();
//#else
//	//// 创建视图器并添加几何体
//	//osgViewer::Viewer viewer;
//	//osg::ref_ptr<osgGA::TrackballManipulator>manipulator = new osgGA::TrackballManipulator;
//
//	//viewer.setCameraManipulator(manipulator);
//
//	//osg::Matrix MVP = viewer.getCamera()->getViewMatrix() * viewer.getCamera()->getProjectionMatrix();
//
//
//	//// 创建一个点
//	//osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
//	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f)*MVP);
//	//vertices->push_back(osg::Vec3f(0.5, 0.5, 0.5));
//	////osg::Vec4f cc = MVP * osg::Vec4f(0.5, 1, 0, 1);
//	////osg::Vec4f cc2 = MVP * osg::Vec4f(0.5, 0, 0, 1);
//	////vertices->push_back(osg::Vec3f(cc.x(), cc.y(), cc.z()));
//	////vertices->push_back(osg::Vec3(cc2.x(), cc2.y(), cc2.z()));
//	//// 创建几何体
//	//osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
//	//geometry->setVertexArray(vertices);
//	//geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
//
//
//	//// 添加着色器程序到几何体
//	//geometry->setUseDisplayList(false);
//	//geometry->setUseVertexBufferObjects(true);
//
//	//// 创建着色器程序并添加几何着色器
//	//osg::ref_ptr<osg::Program> program = new osg::Program;
//	//osg::ref_ptr<osg::Shader> vertShader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource3);
//	//program->addShader(vertShader.get());
//	////osg::ref_ptr<osg::Shader> geoShader = new osg::Shader(osg::Shader::GEOMETRY, geometryShaderSource1);
//	////program->addShader(geoShader.get());
//	//osg::ref_ptr<osg::Shader> fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource2);
//	//program->addShader(fragShader.get());
//	//osg::ref_ptr<osg::StateSet> stateSet = geometry->getOrCreateStateSet();
//
//	////osg::Matrix worldMatrix = osg::Matrix::identity();
//	////worldMatrix *= osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Z_AXIS);
//
//	////osg::ref_ptr<osg::Uniform> transformUniform = new osg::Uniform("transform", osg::Matrix());
//	////stateSet->addUniform(transformUniform);
//	//stateSet->setAttributeAndModes(program.get());
//
//
//	//osg::ref_ptr<osg::Geode> geode1 = new osg::Geode;
//	//geode1->addChild(geometry);
//
//
//	//// 创建一个几何节点用于绘制坐标系
//	//osg::ref_ptr<osg::Geode> geode = new osg::Geode;
//
//	//// 创建X轴线段
//	//osg::ref_ptr<osg::Geometry> xAxisGeom = new osg::Geometry;
//	//osg::ref_ptr<osg::Vec3Array> xAxisVertices = new osg::Vec3Array;
//	//xAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	//xAxisVertices->push_back(osg::Vec3(1.0, 0.0, 0.0));
//	//xAxisGeom->setVertexArray(xAxisVertices);
//	//osg::ref_ptr<osg::Vec4Array> xAxisColors = new osg::Vec4Array;
//	//xAxisColors->push_back(osg::Vec4(1.0, 0.0, 0.0, 1.0)); // 红色
//	//xAxisGeom->setColorArray(xAxisColors, osg::Array::BIND_OVERALL);
//	//xAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	//osg::ref_ptr<osg::LineWidth> xAxisLineWidth = new osg::LineWidth(3.0f);
//	//xAxisGeom->getOrCreateStateSet()->setAttributeAndModes(xAxisLineWidth, osg::StateAttribute::ON);
//
//
//	//// 创建y轴线段
//	//osg::ref_ptr<osg::Geometry> yAxisGeom = new osg::Geometry;
//	//osg::ref_ptr<osg::Vec3Array> yAxisVertices = new osg::Vec3Array;
//	//yAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	//yAxisVertices->push_back(osg::Vec3(0.0, 1.0, 0.0));
//	//yAxisGeom->setVertexArray(yAxisVertices);
//	//osg::ref_ptr<osg::Vec4Array> yAxisColors = new osg::Vec4Array;
//	//yAxisColors->push_back(osg::Vec4(0.0, 0.0, 1.0, 1.0)); // 红色
//	//yAxisGeom->setColorArray(yAxisColors, osg::Array::BIND_OVERALL);
//	//yAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	//osg::ref_ptr<osg::LineWidth> yAxisLineWidth = new osg::LineWidth(3.0f);
//	//yAxisGeom->getOrCreateStateSet()->setAttributeAndModes(yAxisLineWidth, osg::StateAttribute::ON);
//
//
//
//	//// 创建z轴线段
//	//osg::ref_ptr<osg::Geometry> zAxisGeom = new osg::Geometry;
//	//osg::ref_ptr<osg::Vec3Array> zAxisVertices = new osg::Vec3Array;
//	//zAxisVertices->push_back(osg::Vec3(0.0, 0.0, 0.0));
//	//zAxisVertices->push_back(osg::Vec3(0.0, 0.0, 1.0));
//	//zAxisGeom->setVertexArray(zAxisVertices);
//	//osg::ref_ptr<osg::Vec4Array> zAxisColors = new osg::Vec4Array;
//	//zAxisColors->push_back(osg::Vec4(0.0, 1.0, 0.0, 1.0)); // 红色
//	//zAxisGeom->setColorArray(zAxisColors, osg::Array::BIND_OVERALL);
//	//zAxisGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
//	//osg::ref_ptr<osg::LineWidth> zAxisLineWidth = new osg::LineWidth(3.0f);
//	//zAxisGeom->getOrCreateStateSet()->setAttributeAndModes(zAxisLineWidth, osg::StateAttribute::ON);
//
//
//
//
//	//geode->addDrawable(xAxisGeom);
//	//geode->addDrawable(yAxisGeom);
//	//geode->addDrawable(zAxisGeom);
//
//	////
//	////	// 创建顶点数据
//	////	osg::Vec3Array* vertices = new osg::Vec3Array;
//	////	vertices->push_back(osg::Vec3(-0.5f, -0.5f, 0.0f));
//	////	vertices->push_back(osg::Vec3(0.5f, -0.5f, 0.0f));
//	////	vertices->push_back(osg::Vec3(0.5f, 0.5f, 0.0f));
//	////	vertices->push_back(osg::Vec3(-0.5f, 0.5f, 0.0f));
//	////
//	////	// 创建索引数据
//	////	osg::DrawElementsUInt* indices = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
//	////	indices->push_back(0);
//	////	indices->push_back(1);
//	////	indices->push_back(2);
//	////	indices->push_back(3);
//	////
//	////	// 创建几何体
//	////	osg::Geometry* geometry = new osg::Geometry;
//	////	geometry->setVertexArray(vertices);
//	////	geometry->addPrimitiveSet(indices);
//	////
//	//////	geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));
//	////
//	////	// 创建着色器程序
//	////	osg::Shader* vertexShader = new osg::Shader(osg::Shader::VERTEX, R"(
//	////        #version 330
//	////        in vec3 vertexPosition;
//	////        uniform mat4 MVP;
//	////        void main()
//	////        {
//	////            gl_Position = MVP * vec4(vertexPosition, 1.0);
//	////        }
//	////    )");
//	////
//	////	osg::Shader* fragmentShader = new osg::Shader(osg::Shader::FRAGMENT, R"(
//	////        #version 330
//	////        out vec4 fragColor;
//	////        void main()
//	////        {
//	////            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
//	////        }
//	////    )");
//	////
//	////	osg::Program* program = new osg::Program;
//	////	program->addShader(vertexShader);
//	////	program->addShader(fragmentShader);
//	////
//	////
//	////	// 设置几何体的着色器程序
//	////	//osg::ref_ptr<osg::Uniform> transformUniform = new osg::Uniform("MVP", osg::Matrix());
//	////
//	////	osg::ref_ptr<osg::StateSet> stateSet = geometry->getOrCreateStateSet();
//	////	//stateSet->addUniform(transformUniform);
//	////	stateSet->setAttributeAndModes(program, osg::StateAttribute::ON);
//	////	//geometry->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);
//	////	//	osg::ref_ptr<osg::Uniform> transformUniform = new osg::Uniform("transform", osg::Matrix());
//	//////	stateSet->addUniform(transformUniform);
//	////	// 创建节点并添加几何体
//	////	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
//	////	geode->addDrawable(geometry);
//	////
//	//	// 创建一个根节点
//	//osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
//	//// 将几何节点添加到根节点
//	//root->addChild(geode1);
//	//root->addChild(geode);
//
//
//
//	//viewer.setSceneData(root);
//	////osg::Matrix MVP = viewer.getCamera()->getViewMatrix() * viewer.getCamera()->getProjectionMatrix();
//	////osg::ref_ptr<osg::Uniform> transformUniform = new osg::Uniform("MVP", MVP);
//	////stateSet->addUniform(transformUniform);
//	////while (!viewer.done())
//	////{
//	////	// 获取MVP矩阵并传递给顶点着色器的uniform变量
//	////	MVP = viewer.getCamera()->getProjectionMatrix()*viewer.getCamera()->getViewMatrix();
//	////	transformUniform->set(MVP);
//	////	//program->addBindUniformBlock(new osg::Uniform("MVP", MVP));
//	////	//stateSet->addUniform(transformUniform);
//	////	viewer.frame();
//	////}
//	////	osg::Matrix MVP = viewer.getCamera()->getViewMatrix() * viewer.getCamera()->getProjectionMatrix();
//	////
//	//////	transformUniform->set(MVP);
//	////
//	////		// 设置几何体的着色器程序
//	////
//	////	////osg::ref_ptr<osg::StateSet> stateSet = geometry->getOrCreateStateSet();
//	////	stateSet->addUniform(transformUniform);
//	//viewer.realize();
//	////viewer.getCamera()->getGraphicsContext()->getState()->resetVertexAttributeAlias(false);//1
//	////viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);//2
//	////viewer.getCamera()->getGraphicsContext()->getState()->setUseVertexAttributeAliasing(true);//3
//	//return viewer.run();
//	//return 0;
//	float scale = 1.0;
//	osgViewer::Viewer viewer;
//	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile("H:/thirdparty/osg/OpenSceneGraph-Data-3.4.0/OpenSceneGraph-Data/glider.osg");
//	osg::ref_ptr<osgManipulator::Selection> selection = new osgManipulator::Selection;
//	selection->addChild(loadedModel);
//
//	//盒式拖拽器
//	osgManipulator::TabBoxDragger* tad = new osgManipulator::TabBoxDragger();
//	tad->setupDefaultGeometry();//使用默认的图形，即一个立方体
//	tad->setMatrix(osg::Matrix::scale(scale, scale, scale)*osg::Matrix::translate(loadedModel->getBound().center()));
//	tad->addTransformUpdating(selection);//Dragger要改变的对象。
//	tad->setHandleEvents(true);
//
//	//旋转拖拽器
//	osgManipulator::TrackballDragger* dragger = new osgManipulator::TrackballDragger();
//	dragger->setupDefaultGeometry();
//	dragger->setMatrix(osg::Matrix::scale(scale*0.5, scale*0.5, scale*0.5)*osg::Matrix::translate(loadedModel->getBound().center()));
//	dragger->addTransformUpdating(selection);
//	dragger->setHandleEvents(true);
//
//	//三维平移拖拽器
//	osgManipulator::TranslateAxisDragger * Move3d = new osgManipulator::TranslateAxisDragger();
//	Move3d->setupDefaultGeometry();
//	Move3d->setMatrix(osg::Matrix::scale(scale, scale, scale)*osg::Matrix::translate(loadedModel->getBound().center()));
//	Move3d->addTransformUpdating(selection);
//	Move3d->setHandleEvents(true);
//
//	osgManipulator::RotateCylinderDragger *rote = new osgManipulator::RotateCylinderDragger();
//	rote->setupDefaultGeometry();
//	rote->setMatrix(osg::Matrix::scale(scale, scale, scale)*osg::Matrix::translate(loadedModel->getBound().center()));
//	rote->addTransformUpdating(selection);
//	rote->setHandleEvents(true);
//
//	osg::ref_ptr<osg::Group> root = new osg::Group;
//	root->addChild(selection);
//	root->addChild(dragger);
//	//root->addChild(tad);
//	root->addChild(Move3d);
//	//root->addChild(rote);
//
//	viewer.setSceneData(root.get());
//	return viewer.run();
//#endif
//}
#endif
