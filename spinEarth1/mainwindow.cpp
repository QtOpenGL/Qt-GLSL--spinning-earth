#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QOpenGLWidget(parent), m_iSpinAngle(0)
{
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
}

MainWindow::~MainWindow()
{
    m_timer.stop();
    m_pTextures->release();
    delete m_pTextures;
    delete m_pProgram;
    delete [] m_pVertices;

}

void MainWindow::initializeGL()
{
    initializeOpenGLFunctions();
    m_iCount = 0;
    m_uiTexNum = 0;
    QImage img(QString("earth.bmp"));
    //给顶点赋值
    int iAngleStep = 10, iR = 1;
    m_pVertices = new GLfloat[180 / iAngleStep * 360 / iAngleStep * 6 * 3];
    for(int iLongitude = 0; iLongitude < 360; iLongitude += iAngleStep)
    {
        for(int iLatitude = 0; iLatitude < 180; iLatitude += iAngleStep)
        {
            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude + iAngleStep;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude + iAngleStep;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;

            m_pVertices[m_iCount++] = iLongitude;
            m_pVertices[m_iCount++] = iLatitude;
            m_pVertices[m_iCount++] = iR;
        }
    }

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
                        "#version 330\n"
                        "in vec3 pos;\n"
                        "out vec2 texCoord;\n"
                        "uniform mat4 mat4MVP;\n"
                        "void main()\n"
                        "{\n"
                        "    float DEG2RAD = 3.1415926 / 180.0;\n"
                        "    float x = pos[2] * sin(pos[1] * DEG2RAD) * cos(pos[0] * DEG2RAD);\n"
                        "    float y = pos[2] * sin(pos[1] * DEG2RAD) * sin(pos[0] * DEG2RAD);\n"
                        "    float z = pos[2] * cos(pos[1] * DEG2RAD);\n"
                        "    gl_Position = mat4MVP * vec4(x, y, z, 1.0);\n"
                        "    texCoord = vec2(pos.x / 360.0, 1.0 - pos.y / 180.0);\n"//纹理的Y方向是从下向上的，而pos.y的正方向是从上向下，所以是1.0 - pos.y / 180.0
                        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
                        "#version 330\n"
                        "out vec4 color;\n"
                        "in vec2 texCoord;\n"
                        "uniform sampler2D Tex\n;"
                        "void main()\n"
                        "{\n"
                        "    color = texture(Tex, texCoord);\n"//注意，texCoord的值域在0-1之间
                        "}\n";
    fshader->compileSourceCode(fsrc);

    m_pProgram = new QOpenGLShaderProgram;
    m_pProgram->addShader(vshader);
    m_pProgram->addShader(fshader);
    m_pProgram->link();
    m_pProgram->bind();

    m_uiVertLoc = m_pProgram->attributeLocation("pos");
    m_pProgram->enableAttributeArray(m_uiVertLoc);
    m_pProgram->setAttributeArray(m_uiVertLoc, m_pVertices, 3, 0);


    m_pTextures = new QOpenGLTexture(img.mirrored());
    m_pTextures->setMinificationFilter(QOpenGLTexture::Nearest);
    m_pTextures->setMagnificationFilter(QOpenGLTexture::Linear);
    m_pTextures->setWrapMode(QOpenGLTexture::Repeat);
    m_pProgram->setUniformValue("Tex", m_uiTexNum);

    //qDebug()<<m_iCount;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0,0,0,1);

    m_timer.start(100);
}

void MainWindow::paintGL()
{
    //QMatrix4x4在声明时被默认为单位矩阵
    QMatrix4x4 m1, m2, m3, m;

    m1.ortho(-1.0f, +1.0f, -1.0f, 1.0f, -50.0f, 50.0f);//right//generate projection matrix
    m2.lookAt(QVector3D(2,0,0), QVector3D(0,0,0), QVector3D(0,0,10));//generate view matrix, right
    //qDebug()<<m2;
    //m3.translate(0,-0.707,0.0);//right, generate model matrices
    m3.rotate(m_iSpinAngle, 0.0f, 0.0f, 1.0f);//right, generate model matrices

    m = m1 * m2 * m3;

    m_pProgram->setUniformValue("mat4MVP", m);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pTextures->bind(m_uiTexNum);
    glDrawArrays(GL_TRIANGLES, 0, m_iCount / 3);
    m_pTextures->release();
}

void MainWindow::resizeGL(int w, int h)
{
    glViewport(0,0,w,h);
}

void MainWindow::OnTimer(void)
{
    m_iSpinAngle += 5;
    if(360 == m_iSpinAngle)
    {
        m_iSpinAngle = 0;
    }

    update();
}
