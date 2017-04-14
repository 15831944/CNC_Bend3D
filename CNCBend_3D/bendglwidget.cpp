#include "bendglwidget.h"
#include <QDebug>
#include <math.h>
#include <QProgressDialog>
#include "print3d.h"
#include <math.h>
#include "sysglobalvars.h"
#include "progressdlg.h"

//设置与光源有关的数组
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat mat_shininess[] = { 50.0 };
GLfloat light_position[] = {0.0,0.0,0.0,1.0};
GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat Light_Model_Ambient[] = {1.0,1.0,1.0,0.8};
//GLfloat matSpecular[4] = {1.0,1.0,1.0,1.0};
//GLfloat matShininess[] = {50.0};
GLfloat lightAmbient[4] = {0.1,0.1,0.1,1.0};    //环境光，环境光来自四面八方，所有场景中的对象都处于环境光的照射中
GLfloat lightDiffuse[4] = {0.2,0.2,0.2,1.0};    //漫射光，漫射光由特定的光源产生，并在场景中的对象表面产生反射
GLfloat lightPosition[4] = {0.0,0.0,100.0,1.0};

bendGLWidget::bendGLWidget(QWidget *parent) :
    QGLWidget(parent)
{
    setGeometry(0,0,parentWidget()->width(),parentWidget()->height());

    pDrawData = NULL;
    xRote = 0.0;
    yRote = 0.0;
    zRote = 0.0;
    zoom = 1.0;
    hMove = 0.0;
    vMove = 0.0;
    printFlag = false;
    curNum = 0;

    ex = 0.0;
    ey = 0.0;
    ez = circle_radius;
    isAxisBeside=false;
}

bendGLWidget::~bendGLWidget()
{
}

/********************************************
 *function:根据窗口的大小改变绘图区域
 *adding:
 *author: xu
 *date: 2015/9/18
 *******************************************/
void bendGLWidget::resizeWindow(int width, int height)
{
    setGeometry(0,0,width,height);
    updateGL();
}

/********************************************
 *function:旋转图形
 *adding:
 *author: xu
 *date: 2015/10/10
 *******************************************/
void bendGLWidget::rotateGraph(float xrote, float yrote, float zrote)
{
    xRote += xrote;
    yRote += yrote;
    zRote += zrote;
    updateGL();
}

/********************************************
 *function:变换图形
 *adding:58
 *
 *author: xu
 *date: 2015/9/21
 *******************************************/
void bendGLWidget::convertGraph(float para, CONVERT convert)
{
    switch (convert) {
    case CONVERT_HORIZONTAL://水平移动
    {
        hMove += para;
        updateGL();
        break;
    }
    case CONVERT_VERTICAL:  //垂直移动
    {
        vMove += para;
        updateGL();
        break;
    }
    case CONVERT_ZOOM:      //缩放
    {
        zoom += para;
        if(zoom < 0.1)
            zoom = 0.1;
        if(zoom > 1.3)
            zoom = 1.3;
        updateGL();
        break;
    }
    case CONVERT_X:         //绕X轴旋转
    {
        xRote += para;
        updateGL();
        break;
    }
    case CONVERT_Y:          //绕Y轴旋转
    {
        yRote += para;
        updateGL();
        break;
    }
    case CONVERT_Z:         //绕Z轴旋转GLfloat machineColor[4] = {0.1,0.1,0.1,1.0};    //机床本体颜色
    {
        zRote += para;
        updateGL();
        break;
    }
    default:
        break;
    }
}

/********************************************
 *function:复位图形
 *adding:
 *author: xu
 *date: 2015/9/21
 *******************************************/
void bendGLWidget::resetGraph()
{
    xRote = 0.0;
    yRote = -90.0;
    zRote = 0.0;
    zoom = 0.8;
    hMove = 0.0;
    vMove = 0.0;
    updateGL();
}

/********************************************
 *function:将板料模型数据传到OpenGL显示类中
 *adding:
 *author: xu
 *date: 2016/06/18
 *******************************************/
void bendGLWidget::setShowModel(fileOperate *pData)
{
    pDrawData = pData;
    updateGL();
}

void bendGLWidget::formBendImage(QString strName)
{
    int width = this->width();
    int height = this->height();
    const size_t pitch = (width*3+3)&~3; // DWORD aligned line
    const size_t len = pitch*height*sizeof(GLbyte);

    GLbyte* buffer = (GLbyte*)malloc(len);
    if(!buffer)
    {
        fprintf(stderr, "buffer alloc failed!\n");
        return;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, buffer);

    //QString path = QString("D:/%1_2.bmp").arg(count);
    QByteArray ba = strName.toLatin1();
    char *ch = ba.data();
    Print3D print3d;
    print3d.snapshot(width,height,buffer,ch);
}

void bendGLWidget::setPrintMode(bool flag, int num)
{
    printFlag = flag;
    curNum = num;
}

/********************************************
 *function:到轴侧视角进行看图
 *input:无
 *output:无
 *adding:
 *author: wang
 *date: 2017-3-30 15:19:59
 *******************************************/
void bendGLWidget::axisBesideView(point3f a)
{

    hMove = 0-a.x-150; //减是向右移动
    vMove = 0-230;       //减是向上移动
    zMove =300;          //正数是靠近机床方向
    zoom=1;
    isAxisBeside=true;
   updateGL();
}

/********************************************
 *function:对OPENGL进行初始化的设置
 *adding:
 *author: xu
 *date: 2015/9/18
 *******************************************/
void bendGLWidget::initializeGL()
{
    //启用阴影平滑，阴影平滑通过多边形精细的混合色彩，并对外部光进行光滑。
    glShadeModel(GL_SMOOTH);
    //设置清楚屏幕时所用的颜色
    //glClearColor(0.0,0.0,0.0,0.0);
    glClearColor(0.90,0.90,0.90,0.0);
    //设置深度缓存
    glClearDepth(1.0);
    //启用深度测试

    glEnable(GL_MAP2_VERTEX_3);
    glEnable(GLU_MAP1_TRIM_2);
    glEnable(GLU_MAP1_TRIM_3);
    //所作深度测试的类型
    glDepthFunc(GL_LEQUAL);
    //glDepthRange(0.0,1.0);
    //真正精细的透视修正，这一行告诉OPENGL我们希望进行最好的透视修正
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
    //设置光源
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,Light_Model_Ambient);//全局光照

    glLightfv(GL_LIGHT0,GL_AMBIENT,lightAmbient);   //设置环境光的发光亮
    glLightfv(GL_LIGHT0,GL_DIFFUSE,lightDiffuse);   //设置漫射光的发光亮
    glLightfv(GL_LIGHT0,GL_SPECULAR,lightDiffuse);
    GLfloat position[] = {-10.0,10.0,0.0,0.0};
    glLightfv(GL_LIGHT0,GL_POSITION,position);
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);  //打开光照渲染处理
    //启动深度测试
    glEnable(GL_DEPTH_TEST);
    //设置材质
//    glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
//    glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    //glLightModeli(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
}

/********************************************
 *function:包含所有的绘图代码
 *adding:
 *author: xu
 *date: 2015/9/18
 *******************************************/
void bendGLWidget::paintGL()
{
     if(isAxisBeside)
     {
         //清除屏幕和深度缓存
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          //重置当前的模型观察矩阵
          glLoadIdentity();
          gluLookAt(ex, ey, ez,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

         glTranslatef(0.0+hMove,0.0+vMove,0.0+zMove);
         glRotatef(0,1.0,0.0,0.0);
         glRotatef(0,0.0,1.0,0.0);
         glRotatef(0,0.0,0.0,1.0);

        glTranslatef(-hMove,-vMove,-zMove);

        glRotatef(-70,0,1,0);
        glTranslatef(hMove,vMove,zMove);

         if(pDrawData){
         drawScene();
          pDrawData->drawMachine();
           pDrawData->drawWorkPiece();
        }
         glFlush();
         isAxisBeside=false;

//             qDebug()<<"侧视矩阵：";
         GLfloat mat[16];
         glGetFloatv(GL_MODELVIEW_MATRIX, mat);
//             for(int i=0;i<4;i++)
//             {
//                 qDebug()<<mat[i] <<":"<<mat[i+4]<<mat[i+8]<<mat[i+12];
//             }
         hMove=mat[12];
         vMove=mat[13];
         zMove=mat[14]+circle_radius;
         xRote=0;
         yRote=-70;
         zRote=0;

     }
     else
     {
         //清除屏幕和深度缓存
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
          //重置当前的模型观察矩阵
         glLoadIdentity();
         gluLookAt(ex, ey, ez,0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

         glTranslatef(0.0+hMove,0.0+vMove,0.0+zMove);
         qDebug()<<"zMove:"<<zMove;

         glRotatef(xRote,1.0,0.0,0.0);
         glRotatef(yRote,0.0,1.0,0.0);
         glRotatef(zRote,0.0,0.0,1.0);

         glScalef(zoom,zoom,zoom);
         if(pDrawData){
            drawScene();
            pDrawData->drawMachine();
            pDrawData->drawWorkPiece();
            if(printFlag)
            {
                QString name = QString("%1/%2.bmp").arg(picPath).arg(curNum);
                formBendImage(name);
            }
        }
        glFlush();
      }
}

/********************************************
 *function:为透视图设置屏幕
 *adding:
 *author: xu
 *date: 2015/9/18
 *******************************************/
void bendGLWidget::resizeGL(int width, int height)
{
    //防止height为0
    if(0 == height)
        height = 1;
    //重置当前的视口
    glViewport(0,0,width,height);
    //选择投影矩阵
    glMatrixMode(GL_PROJECTION);
    //重置投影矩阵
    glLoadIdentity();
    //建立透视投影矩阵
    //gluPerspective(45.0,(GLfloat)width/(GLfloat)height,1.0,100.0);
    gluPerspective(45.0,(GLfloat)width/(GLfloat)height,1.0,100000.0);
    //选择模型观察矩阵
    glMatrixMode(GL_MODELVIEW);
    //重置模型观察矩阵
    glLoadIdentity();
}
void bendGLWidget::drawScene()
{
    glBegin(GL_LINES);
    glEnable(GL_LINE_SMOOTH);
    glColor4f(0.4,0,0,0);
    int y1 = -1000;
    for(int z=-25;z<25;z++)
    {
        glVertex3f(-7500,y1,z*300);
        glVertex3f(7500,y1,z*300);
    }
    for(int x=-25;x<25;x++)
    {
        glVertex3f(x*300,y1,-7500);
        glVertex3f(x*300,y1,7500);
    }
    glEnd();
}
