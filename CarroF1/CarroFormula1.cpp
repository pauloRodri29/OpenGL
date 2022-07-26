
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>
#pragma warning(disable: 4996)

GLfloat angleX = 0;
GLfloat angleY = 0;
GLuint cam = 0;
GLfloat fAspect = 1;
GLdouble girar = 0;//variaveis de direcionamento
GLdouble acel;//variaveis de aceleração

typedef struct vertex
{
	float x;
	float y;
	float z;
} vertex;

typedef struct face
{
	int v1, v2, v3;
	int n1, n2, n3;
	int t1, t2, t3;
} face;

typedef struct model
{
	std::vector<face> faces;
	std::vector<vertex> vertices;
	std::vector<vertex> normals;
	std::vector<vertex> textures;
} model;

model body, tyre_front, tyre_back;

void loadObj(const char* filename, model& model)
{
	FILE* file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("Erro ao abrir o arquivo!\n");
		return;
	}

	while (1)
	{

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		if (strcmp(lineHeader, "v") == 0)
		{
			vertex v;
			fscanf(file, "%f %f %f\n", &v.x, &v.y, &v.z);
			model.vertices.push_back(v);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			vertex n;
			fscanf(file, "%f %f %f\n", &n.x, &n.y, &n.z);
			model.normals.push_back(n);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			vertex vt;
			fscanf(file, "%f %f\n", &vt.x, &vt.y);
			model.textures.push_back(vt);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			face f;
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &f.v1, &f.t1, &f.n1,
				&f.v2, &f.t2, &f.n2, &f.v3, &f.t3, &f.n3);
			model.faces.push_back(f);
		}
	}
}

GLuint loadTGA(const char* filename)
{

	static GLubyte TGA_TYPE_HEADER[8] = { 0, 0, 2, 0, 0, 0, 0, 0 };
	GLuint width, height, format;

	GLubyte* data;

	FILE* file;
	file = fopen(filename, "rb");

	if (file == NULL)
	{
		printf("Arquivo de textura não encontrado!\n");
		return 0;
	}

	GLubyte type_header[8];
	if (fread(type_header, 1, sizeof(type_header), file) != sizeof(type_header))
	{
		printf("Problema ao ler cabeçalho do arquivo!\n");
		fclose(file);
		return 0;
	}

	if (memcmp(TGA_TYPE_HEADER, type_header, sizeof(type_header)) != 0)
	{
		printf("Tipo de arquivo TGA inválido!\n");
		fclose(file);
		return 0;
	}

	GLubyte image_header[10];
	if (fread(image_header, 1, sizeof(image_header), file) != sizeof(image_header))
	{
		printf("Problema ao ler formato da imagem!\n");
		fclose(file);
		return 0;
	}

	width = image_header[5] * 256u + image_header[4];
	height = image_header[7] * 256u + image_header[6];

	if ((width <= 0) || (height <= 0))
	{
		printf("Tamanho de arquivo inválido!\n");
		fclose(file);
		return 0;
	}

	GLubyte depth = image_header[8];
	switch (depth)
	{
	case 24:
		format = GL_RGB;
		break;
	case 32:
		format = GL_RGBA;
		break;
	default:
		printf("Canal de cor inválido!\n");
		fclose(file);
		return 0;
	}

	size_t pixel_size = depth / 8u;
	size_t data_size = width * height * pixel_size;
	data = (GLubyte*)malloc(data_size);
	if (!data)
	{
		printf("Erro ao alocar memória!\n");
		fclose(file);
		return 0;
	}

	if (fread(data, 1, data_size, file) != data_size)
	{
		printf("Erro ao ler dados da imagem!\n");
		free(data);
		fclose(file);
		return 0;
	}

	// TGA stores image data as BGR(A).
	for (size_t i = 0; i < data_size; i += pixel_size)
	{
		GLubyte temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	fclose(file);

	GLuint idTextura;
	glGenTextures(1, &idTextura);
	glBindTexture(GL_TEXTURE_2D, idTextura);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
		GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);

	return idTextura;
}

void renderModel(const model& model)
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < model.faces.size(); ++i)
	{
		const vertex& v1 = model.vertices[model.faces[i].v1 - 1];
		const vertex& v2 = model.vertices[model.faces[i].v2 - 1];
		const vertex& v3 = model.vertices[model.faces[i].v3 - 1];

		const vertex& n1 = model.normals[model.faces[i].n1 - 1];
		const vertex& n2 = model.normals[model.faces[i].n2 - 1];
		const vertex& n3 = model.normals[model.faces[i].n3 - 1];

		const vertex& t1 = model.textures[model.faces[i].t1 - 1];
		const vertex& t2 = model.textures[model.faces[i].t2 - 1];
		const vertex& t3 = model.textures[model.faces[i].t3 - 1];

		glNormal3f(n1.x, n1.y, n1.z);
		glTexCoord2f(t1.x, t1.y);
		glVertex3f(v1.x, v1.y, v1.z);

		glNormal3f(n2.x, n2.y, n2.z);
		glTexCoord2f(t2.x, t2.y);
		glVertex3f(v2.x, v2.y, v2.z);

		glNormal3f(n3.x, n3.y, n3.z);
		glTexCoord2f(t3.x, t3.y);
		glVertex3f(v3.x, v3.y, v3.z);
	}
	glEnd();
}

void displayPersp(void)
{
	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	{
		glRotatef(angleX, 1, 0, 0);
		glRotatef(angleY, 0, 1, 0);

		renderModel(body);

		glPushMatrix();//Front

		glPushMatrix();
		{
		glTranslatef(-1.1, 0, 0.7);//Roda esquerda   
		glRotated(girar, 0, 1, 0);
		glRotated(acel, 0, 0, 1);
		renderModel(tyre_front);
		}
		glPopMatrix();

		glPushMatrix();
		{
		glTranslatef(-1.1, 0, -0.6);//Roda direita  
		glRotated(girar, 0, 1, 0);
		glRotated(acel, 0, 0, 1);
		glRotatef(180, 1, 0, 0);
		renderModel(tyre_front);
		}
		glPopMatrix();

		glPopMatrix();//Front


		glPushMatrix();
		{
			glTranslatef(2.0, 0, 0.75);//Roda esquerda
			glRotated(acel, 0, 0, 1);
			renderModel(tyre_back);
		}
		glPopMatrix();

		glPushMatrix();
		{
			glTranslatef(2.0, 0, -0.7);//Roda direita
			glRotated(acel, 0, 0, 1);
			glRotatef(180, 1, 0, 0);
			renderModel(tyre_back);
		}
		glPopMatrix();
	}
	glPopMatrix();

	glutSwapBuffers();
}

void init()
{
	loadTGA("f1.tga");

	loadObj("f1.obj", body);
	loadObj("tyre_front.obj", tyre_front);
	loadObj("tyre_back.obj", tyre_back);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	GLfloat luzAmbiente[4] = { 0.3,0.3,0.3,1.0 };
	GLfloat luzDifusa[4] = { 0.5,0.5,0.5,1.0 };		  // "cor"
	GLfloat luzEspecular[4] = { 0.0, 0.0, 0.0, 1.0 }; // "brilho"

	GLfloat posicaoLuz[4] = { 0.0, 200.0, 0.0, 0.0 };// ultimo parâmeto indica se é pontual (1.0) ou direcional (0.0)

	// Capacidade de brilho do material
	GLfloat especularidade[4] = { 1.0,1.0,1.0,1.0 };
	GLint especMaterial = 60;

	// Especifica que a cor de fundo da janela será branca
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Habilita o modelo de colorização de Gouraud
	glShadeModel(GL_SMOOTH);//GL_FLAT

	// Define o grau de especularidade do material
	glMaterialfv(GL_FRONT, GL_SPECULAR, especularidade);

	// Define a concentração do brilho
	glMateriali(GL_FRONT, GL_SHININESS, especMaterial);

	// Ativa o uso da luz ambiente
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

	// Define os parâmetros da luz de número 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);

	// Habilita a definição da cor do material a partir da cor corrente
	glEnable(GL_COLOR_MATERIAL);

	//Habilita o uso de iluminação
	glEnable(GL_LIGHTING);

	// Habilita a luz de número 0
	glEnable(GL_LIGHT0);

	// Habilita o z-buffer
	glEnable(GL_DEPTH_TEST);
}

void resize(GLsizei w, GLsizei h)
{
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);

	fAspect = (GLfloat)w / (GLfloat)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, fAspect, 1, 100);
	gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

void specialKeys(int key, int x, int y)
{

	switch (key)
	{
	case GLUT_KEY_LEFT:
		angleY -= 5;
		break;
	case GLUT_KEY_RIGHT:
		angleY += 5;
		break;
	case GLUT_KEY_UP:
		angleX += 5;
		break;
	case GLUT_KEY_DOWN:
		angleX -= 5;
		break;
	}


	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, fAspect, 1, 100);

	if ((cam % 3) == 0)
		gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
	else
	{
		if ((cam % 3) == 1)
			gluLookAt(0, 5, 0, 0, 0, 0, 1, 0, 0);
		else
			gluLookAt(5, 0, 0, 0, 0, 0, 0, 1, 0);
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'x'://x
		girar += 1;
		girar = (girar > 25) ? 25 : girar;
		//acel += 100;
		break;
	case 'z':
		girar -= 1;
		girar = (girar < -25) ? -25 : girar;
		//acel += 100;
		break;
	case 's':
		acel += 200;
		break;
	case 32:
		angleX = 0;
		angleY = 0;
		cam++;
		break;
	case 27:
		exit(0);
	}
	glutPostRedisplay();

}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Avaliacao 03");
	glutDisplayFunc(displayPersp);
	glutReshapeFunc(resize);
	glutSpecialFunc(specialKeys);
	glutKeyboardFunc(keyboard);
	init();
	glutMainLoop();
	return 0;
}