#include <GL/glut.h>
#include <stdio.h>
GLfloat angS=0.0,angP=0.0,mod_angP=0.0;

typedef struct{
	GLubyte *dibujo;
	GLuint bpp;
	GLuint largo;
	GLuint ancho;
	GLuint ID;
}textura;

textura tEspacio;
textura tSol;


int cargaTGA( char *nombre, textura *imagen){
	GLubyte plantillaTGA[12]={0,0,2,0,0,0,0,0,0,0,0,0}; //codigo binario para identificar tga
	GLubyte cabeceraTGA[12];
	GLubyte detallesTGA[6];
	GLuint bytesporpunto;
	GLuint tamanoTGA;
	GLuint temp,i;
	GLuint tipo=GL_RGBA;
	
	FILE *archivo = fopen(nombre,"rb"); //parte de la funcion stdio, rb significa lectura binaria
	if(archivo == NULL || //hay algun archivo para leer
	fread(cabeceraTGA,1,sizeof(cabeceraTGA),archivo)!=sizeof(cabeceraTGA) || //hay 12 bits para leer?
	memcmp(plantillaTGA,cabeceraTGA,sizeof(cabeceraTGA))!=0 || //son iguales la plantilla y la cabecera?
	fread(detallesTGA,1,sizeof(detallesTGA),archivo)!=sizeof(detallesTGA))
	{ 
		if(archivo==NULL){
			printf("No se encontro el archivo %s\n",nombre);
			return 0;
		}else{
			fclose(archivo);
			return 0;
		}
	}
	
	imagen->largo=256*detallesTGA[1]+detallesTGA[0];//256*3+0 = 768 tamaño de la imagen
	imagen->ancho=256*detallesTGA[3]+detallesTGA[2];
	
	if(imagen->largo <= 0 ||
	imagen->ancho <=0 ||
	(detallesTGA[4]!=24 && detallesTGA[4]!=32)){
		printf("Datos invalidos\n");
		fclose(archivo);
		return 0;
	}
	
	imagen->bpp=detallesTGA[4];//24
	bytesporpunto=detallesTGA[4]/8;//3
	tamanoTGA=imagen->largo*imagen->ancho*bytesporpunto;//768 * 768 * 3 = 1769472 / 1024 kb => 1728/1024 = 1.68 MB tamaño
	
	imagen->dibujo = (GLubyte *)malloc(tamanoTGA);//malloc es asignacion de memoria
	if(imagen->dibujo == NULL ||  //si dibujo no existe
	fread(imagen->dibujo,1,tamanoTGA,archivo) != tamanoTGA){ //si no se asignó correctamente
		if(imagen->dibujo != NULL){ 
			printf("Error leyendo imagen\n");
			free(imagen->dibujo);
		}else{
			printf("Error asignando memoria\n");
		}
		fclose(archivo);
		return 0;		
	}
	
	//los tga sn BGR, hay que pasarlos a rgb
	for (i=0; i<(int)tamanoTGA;i+=bytesporpunto){
		temp=imagen->dibujo[i];
		imagen->dibujo[i]=imagen->dibujo[i+2];
		imagen->dibujo[i+2]=temp;
	}
	fclose(archivo);
	
	glGenTextures(1,&imagen[0].ID);
	glBindTexture(GL_TEXTURE_2D,imagen[0].ID);//identificar la textura que será 2d
	if(imagen->bpp==24) tipo=GL_RGB; //si el bbp es 24 entonces es un rgb
	glTexImage2D(GL_TEXTURE_2D,0,tipo,imagen[0].ancho,imagen[0].largo,0,tipo,GL_UNSIGNED_BYTE,imagen[0].dibujo);//imagen[0].largo,0
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

	return 1;
}

void init(){
	GLfloat light_position[]={0,0,0,1};
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_POSITION,light_position);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	glShadeModel(GL_SMOOTH);//mejorar las sombras
	glClearColor(0.0,0.0,0.0,0.0);//reiniciar colores
	glClearDepth(1.0);//borrar profundidad y establecerla en 1
	glEnable(GL_DEPTH_TEST);//comparar diferentes profundidades, en caso uno detrás de otro
	glDepthFunc(GL_LEQUAL);//luego de comparar, permite ver el resultado
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);//corregir perspectiva y que se vea bien
	glEnable(GL_TEXTURE_2D);//habilitar 2d
	if(!cargaTGA("universo.tga",&tEspacio)){
		printf("Error cargando textura\n");
		exit(0);
	}
	if(!cargaTGA("sol.tga",&tSol)){
	printf("Error cargando textura\n");
	exit(0);
	}				
}

void espacio(){
	glDisable(GL_LIGHTING);//deshabilitar luz
	glEnable(GL_TEXTURE_2D);//habilitar textura
		glBindTexture(GL_TEXTURE_2D,tEspacio.ID);
		int x=300;
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);glVertex3d(-x,-x,-x);				
			glTexCoord2f(1,0);glVertex3d(x,-x,-x);				
			glTexCoord2f(1,1);glVertex3d(x,x,-x);	
			glTexCoord2f(0,1);glVertex3d(-x,x,-x);			
		glEnd();	
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);	
}

void sol(){
	glDisable(GL_LIGHTING);//deshabilitar luz
	glEnable(GL_TEXTURE_2D);//habilitar textura
	glRotatef(angS,0,0,1);//rotacion dinámica
	GLUquadric *objq = gluNewQuadric();
	gluQuadricTexture(objq,GL_TRUE);//permite habilitar puntero
	glBindTexture(GL_TEXTURE_2D,tSol.ID);	
		
	gluSphere(objq,30,50,50);		
	gluDeleteQuadric(objq);
	
	glLoadIdentity();
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);	
}

void orbita(GLint dist){
	GLfloat r=(GLfloat)30/255;
	GLfloat g=(GLfloat)144/255;
	GLfloat b=(GLfloat)255/255;
	
	GLfloat mat_ambient[]={r,g,b,0.5};
	GLfloat mat_diffuse[]={r,g,b,0.5};
	GLfloat mat_specular[]={r,g,b,0.5};
	GLfloat mat_shininess[]={50};
	
	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
	
	glutSolidTorus(2,dist,50,50);//grosor,distancia del medio, enmallado,enmallado
	glLoadIdentity();
	
}

void planeta(GLfloat angP, GLint dist,GLint radPlaneta, GLint re,GLint gr,GLint bl,GLint satelite,GLint radSatelite, GLint anillo, GLint cantAnillos ){
	orbita(dist);//crear orbita
	
	glRotated(angP,0,0,1);//rotacion dinámica
	glTranslated(dist,0,0);
	
	GLfloat r=(GLfloat)re/255;
	GLfloat g=(GLfloat)gr/255;
	GLfloat b=(GLfloat)bl/255;
	
	GLfloat mat_ambient[]={r,g,b,1};
	GLfloat mat_diffuse[]={r,g,b,1};
	GLfloat mat_specular[]={r,g,b,1};
	GLfloat mat_shininess[]={50};
	
	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);
	
	glutSolidSphere(radPlaneta,50,50);//grosor,distancia del medio, enmallado,enmallado

	if(anillo==1){//crea un anillo
		//cantidad de anillos
		for(int i=1; i<=cantAnillos; i++){
			glutSolidTorus(0.9,radPlaneta+(i*2),50,50);
		}	
		
	}
	
	if(satelite==1){//crea una luna
		glRotated(angP*2,0,0,1);
		glTranslated(radPlaneta+3,0,0);
		glutSolidSphere(radSatelite,50,50);
	}	

	glLoadIdentity();
	
}

void display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	angP-=mod_angP;
	espacio();	//crear espacio
	sol();//crear sol
	glLoadIdentity();

	//colores reales según la nasa xd
	//planeta(angP * % rotacion, distan.sol , radio planeta,    red , green , blue , con satelite (0,1) ,radio satelite ,  con anillo (0,1), cantidadAnillos)
	planeta(angP*0.9, 40, 5,   221,177,112,   0,0, 0,0);//mercurio
	planeta(angP*0.8, 60, 7,  226,225,221,  0,0, 0,0);//venus
	planeta(angP*0.7, 85, 10,  30,200,255,  1,3, 0,0);//tierra
	planeta(angP*0.6, 115, 10,  250,143,110,  0,0, 0,0);//marte
	planeta(angP*0.5, 155, 20,  220,150,100,  0,0, 1,1);//jupiter
	planeta(angP*0.4, 200, 10,  216,185,121,  0,0, 1,3);//saturno
	planeta(angP*0.3, 235, 8,  195,233,236,  0,0, 1,2);//urano
	planeta(angP*0.2, 265, 7,  68,106,253,  0,0, 1,1);//neptuno
	planeta(angP*0.1, 285, 5,  216,176,140,  0,0, 0,0);//plutón
	
	glutSwapBuffers();//para texturas porque se usa más memoria
}

void reshape(int ancho, int altura){
	glViewport(0,0,(GLsizei)ancho,(GLsizei)altura);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int h=300;
	glOrtho(-h,h,-h,h,-h,h);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle(){
	angS+=0.03;//angulo de rotación cada vez que se ejecuta el loop
	display();
}

void keyboard(unsigned char key, int x, int y){
	switch(key){
		case 27: exit(0);break;
	//	case ' ': mod_angP=0;break;
		case 45: mod_angP-=0.005;break;
		case 43: mod_angP+=0.005;break;
	}
}

void keyboard_s(int key, int x, int y){
	switch(key){
		case GLUT_KEY_F2: glutIdleFunc(idle);break;
		case GLUT_KEY_F1: glutIdleFunc(NULL);break;
	}
}

int main(int argc, char** argv) {
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);//doblre memoria para renderizado para profundidad
	glutInitWindowSize(700,700);
	glutInitWindowPosition(50,50);
	glutCreateWindow("PC3_SistemaSolar_AbelGuerra");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);	
	
	glutKeyboardFunc(keyboard);//teclas
	glutSpecialFunc(keyboard_s);//teclas especiales
	
	glutIdleFunc(idle);		
	glutMainLoop();	
	
	return 0;
}

