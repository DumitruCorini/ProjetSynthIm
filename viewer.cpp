#include "viewer.h"

#include <math.h>
#include <iostream>
#include <QTime>

using namespace std;

Viewer::Viewer(const QGLFormat &format)
  : QGLWidget(format),
    _timer(new QTimer(this)),
    _drawMode(false) {
  _GRID_SIZE = 1024;
  _grid = new Grid(_GRID_SIZE, -1.0f, 1.0f);
  
  // create a camera (automatically modify model/view matrices according to user interactions)
  _cam  = new Camera(1,glm::vec3(0.0f, 0.0f, 0.0f));
  
  _FOG_COLOR = glm::vec4(0.5f,0.5f,0.5f,1.0f);
  _deplacement = glm::vec2(0.0f, 0.0f);


  _deplacement = glm::vec2(0.0f, 0.0f);

  _timer->setInterval(10);
  connect(_timer, SIGNAL(timeout()), this, SLOT(updateGL()));
}

Viewer::~Viewer() {
  // delete everything 
  delete _timer;
  delete _cam;
  delete _grid;

  deleteVAO();
  deleteShaders();
  deleteFBO();
}

void Viewer::createShaders() {

  _vertexFilenames.push_back("shaders/first.vert");
  _fragmentFilenames.push_back("shaders/first.frag");

  _vertexFilenames.push_back("shaders/second.vert");
  _fragmentFilenames.push_back("shaders/second.frag");

  _vertexFilenames.push_back("shaders/fourth.vert");
  _fragmentFilenames.push_back("shaders/fourth.frag");
}

void Viewer::deleteShaders() {
    for (unsigned int i = 0; i < _shaders.size(); i++){
        delete _shaders[i];
    }
}

void Viewer::createVAO() {

  //the variable _grid should be an instance of Grid
  //the .h file should contain the following VAO/buffer ids
  //GLuint _vaoTerrain;
  //GLuint _vaoQuad;
  //GLuint _terrain[2];
  //GLuint _quad;

  glGenBuffers(2, _terrain);//tao 2 VBO

  glGenVertexArrays(1, &_vaoTerrain);//tao 1 VAO id vaoterrain
  // create VAO
  glGenBuffers(1, &_quad);// tao 1 VBO quad
  glGenVertexArrays(1, &_vaoQuad);// tao 1 VAO vaoQuad
  
  // create the VAO associated with the screen quad
  // 2 triangles that cover the viewPort
  const GLfloat quadData[] = {
    -1.0f,-1.0f,0.0f,
     1.0f,-1.0f,0.0f,
    -1.0f,1.0f,0.0f,
    -1.0f,1.0f,0.0f,
     1.0f,-1.0f,0.0f,
     1.0f,1.0f,0.0f
  };

  // create the VBO associated with the grid (the terrain)
  glBindVertexArray(_vaoTerrain);
  glBindBuffer(GL_ARRAY_BUFFER, _terrain[0]); // vertices
  // dua du lieu vao VBO 
  glBufferData(GL_ARRAY_BUFFER, _grid->nbVertices()*3*sizeof(float), _grid->vertices(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _terrain[1]); // faces
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, _grid->nbFaces()*3*sizeof(unsigned int), _grid->faces(), GL_STATIC_DRAW);


  // bind vao and send vertices
  glBindVertexArray(_vaoQuad);
  glBindBuffer(GL_ARRAY_BUFFER, _quad); // vertices
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  // back to normal
  glBindVertexArray(0);
}

void Viewer::deleteVAO() {
  glDeleteBuffers(2, _terrain);
  glDeleteBuffers(1, &_quad);
  glDeleteVertexArrays(1, &_vaoTerrain);
  glDeleteVertexArrays(1, &_vaoQuad);
}

void Viewer::createFBO(){
    glGenFramebuffers(1, &_fbo);
    glGenTextures(1, &_heightMap);
    glGenTextures(1, &_normalMap);
}
\
void Viewer::initFBO() {
  // create the texture for rendering the normal map values
  glBindTexture(GL_TEXTURE_2D, _normalMap);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, _heightMap);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width(), height(), 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // attach textures to framebuffer object
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  glBindTexture(GL_TEXTURE_2D, _normalMap);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _normalMap, 0);
  glBindTexture(GL_TEXTURE_2D, _heightMap);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _heightMap, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      cout << "Erreur a l'initialisation du framebuffer" << endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewer::deleteFBO() {
  // delete all FBO Ids
  glDeleteFramebuffers(1, &_fbo);
  glDeleteTextures(1, &_normalMap);
  glDeleteTextures(1, &_heightMap);
}

void Viewer::drawVAO() {

  // activate the VAO, draw the associated triangles and desactivate the VAO
  glBindVertexArray(_vaoTerrain);
  glDrawElements(GL_TRIANGLES, 3*_grid->nbFaces(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Viewer::drawGrid(unsigned int shader){

    GLuint id = _shaders[shader]->id();

    glActiveTexture(GL_TEXTURE0+0);
    glBindTexture(GL_TEXTURE_2D, _heightMap);
    glUniform1ui(glGetUniformLocation(id, "heightmap"), 0);

    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_2D, _normalMap);
    glUniform1ui(glGetUniformLocation(id, "normalmap"), 0);

    glBindVertexArray(_vaoTerrain);
    glDrawElements(GL_TRIANGLES, 3*_grid->nbFaces(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void Viewer::drawQuad(){
    glBindVertexArray(_vaoQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Viewer::enableShaders(unsigned int shader) {

  // current shader ID
  GLuint id = _shaders[shader]->id();

  // activate the current shader
  glUseProgram(id);

  // send the model-view matrix
  glUniformMatrix4fv(glGetUniformLocation(id,"mdvMat"),1,GL_FALSE,&(_cam->mdvMatrix()[0][0]));

  // send the projection matrix
  glUniformMatrix4fv(glGetUniformLocation(id,"projMat"),1,GL_FALSE,&(_cam->projMatrix()[0][0]));

  //send the fogcolor
  glUniform3fv(glGetUniformLocation(id,"FogColor"),1,&(_FOG_COLOR[0]));

  //mouvement
  glUniform2f(glGetUniformLocation(id,"deplacement"),_deplacement.x,_deplacement.y);
}

void Viewer::disableShaders() {
  // desactivate all shaders 
  glUseProgram(0);
}

void Viewer::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    // a partir de maintenant je dessine dans une texture
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    enableShaders(0);
    //GLenum buffers [] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffer(GL_COLOR_ATTACHMENT1);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // shader that draws grid
    enableShaders(1);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawGrid(1);

    //enable shader post process
    /*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    enableShaders(2);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    drawGrid(2);*/
    
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //glClearColor(0.5f,0.5f,0.5f,1);
    /*
    GLfloat fogColor[] = {0.5f,0.5f,0.5f,1.0f};
    glEnable(GL_FOG);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi( GL_FOG_MODE,GL_EXP2);
    glFogf(GL_FOG_START,10.0f);
    glFogf(GL_FOG_END,20.0f);
    glFogf(GL_FOG_DENSITY,0.3f);
    glFogf(GL_FOG_HINT,GL_NICEST);
    */

    // tell the GPU to stop using this shader
    disableShaders();
}

void Viewer::resizeGL(int width, int height) {
  _cam->initialize(width, height, false);
  glViewport(0,0, width, height);
  initFBO();
  updateGL();
}

void Viewer::mousePressEvent(QMouseEvent *me) {

  // handle camera events
  const glm::vec2 p((float)me->x(),(float)(height()-me->y()));

  if(me->button()==Qt::LeftButton) {
    _cam->initRotation(p);
  } else if(me->button()==Qt::RightButton) {
    _cam->initMoveXY(p);
  } else if(me->button()==Qt::MidButton) {
    _cam->initMoveZ(p);
  }

  updateGL();
}

void Viewer::mouseMoveEvent(QMouseEvent *me) {
  // handle camera motion
  const glm::vec2 p((float)me->x(), (float)(height()-me->y()));
 
  _cam->move(p);
  updateGL();
}

void Viewer::keyPressEvent(QKeyEvent *ke) {
  // key a: play/stop animation
  if(ke->key()==Qt::Key_A){
      _deplacement.x += 0.1 ;
  }else if(ke->key()==Qt::Key_D){
      _deplacement.x -= 0.1;
  }else if(ke->key()==Qt::Key_W){
      _deplacement.y += 0.1;
  }else if(ke->key()==Qt::Key_S){
      _deplacement.y -= 0.1;
  }


  // key i: init camera
  if(ke->key()==Qt::Key_I) {
    _cam->initialize(width(), height(), true);
  }

  // key f: compute FPS
  if(ke->key()==Qt::Key_F) {
    int elapsed;
    QTime timer;
    timer.start();
    unsigned int nb = 500;
    for(unsigned int i=0; i<nb; ++i) {
      paintGL();
    }
    elapsed = timer.elapsed();
    double t = (double)nb/((double)elapsed);
    cout << "FPS : " << t*1000.0 << endl;
  }

  // key r: reload shaders
  if(ke->key()==Qt::Key_R) {
    for(unsigned int i=0; i<_vertexFilenames.size(); ++i) {
      _shaders[i]->reload(_vertexFilenames[i].c_str(), _fragmentFilenames[i].c_str());
    }
  }

  updateGL();
}

void Viewer::initializeGL() {
  // make this window the current one
  makeCurrent();

  glewExperimental = GL_TRUE;

  // init and chack glew
  if(glewInit()!=GLEW_OK) {
    cerr << "Warning: glewInit failed!" << endl;
  }

  // init OpenGL settings
  glClearColor(0.0,0.0,0.0,1.0);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glViewport(0,0, width(), height());
  
  // initialize camera
  _cam->initialize(width(), height(), true);

  // create and initialize shaders and VAO 
  createShaders();

  for(unsigned int i=0;i<_vertexFilenames.size();++i) {
    _shaders.push_back(new Shader());
    _shaders[i]->load(_vertexFilenames[i].c_str(), _fragmentFilenames[i].c_str());
  }

  createVAO();

  createFBO();
  initFBO();

  // starts the timer 
  _timer->start();
}
