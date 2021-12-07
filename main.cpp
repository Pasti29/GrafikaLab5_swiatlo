//*********************************************************************
//
//  PLIK ŹRÓDŁOWY:		main.cpp
//
//  OPIS:				Program do Laboratorium nr 5 rysujący na
//						ekranie obiekt 3-D, który jest oświetlony
//		                dwoma źródłami światła
//
//  AUTOR:				Karol Pastewski, 252798@student.edu.pl
//
//  DATA
//	MODYFIKACJI:        29.11.2021r.
//
//  PLATFORMA:			System operacyjny:  Microsoft Windows 11
//						Środowisko:         Microsoft Visual 2019
//
//  MATERIAŁY			Dokumentacja OpenGL
//	ŹRÓDŁOWE:			Dokumentacja GLUT
//						www.zsk.ict.pwr.wroc.pl
//
//  UŻYTE BIBLIOTEKI	cmath — obsługuje matematyczne wzory i stałe
//  NIESTANDARDOWE
//
//*********************************************************************

#define _USE_MATH_DEFINES

#include <windows.h>
#include <gl/gl.h>
#include <gl/glut.h>
#include <cstdlib>
#include <iostream>
#include <cmath>

using namespace std;

typedef float point3[3];

struct VERTEXES {
    float point3[3];
    float normalized[3];
};

VERTEXES **ARRAY;           // niezainicjalizowana tablica dwuwymiarowa; patrz funkcję 'setArray()'
int N;                      // wielkość tablicy zainicjowana w funkcji 'setArray()'
static int rotationSwitch = 1;
// 1 - wybór obracania obiektem
// 2 - wybór obracania źródłami światła
static int showAxes = 0;
// 0 - brak osi współrzędnych
// 1 - rysowanie osi współrzędnych
static int objectMode = 1;
// 1 - wyświetlenie jajka;
// 2 - wyświetlenie czajnika
static int status = 0;
// stan klawiszy myszy
// 0 - nie naciśnięto żadnego klawisza
// 1 - naciśnięty zostać lewy klawisz
// 2 - naciśnięty prawy klawisz

static float objectRotation[] = {0.0, 0.0};

static float pix2angle;     // przelicznik pikseli na stopnie

static int x_pos_old = 0;       // poprzednia pozycja kursora myszy
static float delta_x = 0;        // różnica pomiędzy pozycją bieżącą i poprzednią kursora myszy

static int y_pos_old = 0;       // poprzednia pozycja kursora myszy
static float delta_y = 0;        // różnica pomiędzy pozycją bieżącą i poprzednią kursora myszy


static float viewerZ = 10.0;

static float radius = 15.0;     // promień

// Parametry położenia obiektu na układzie współrzędnych
static float centerX = 0.0;
static float centerY = 0.0;
static float centerZ = 0.0;

// Parametry pierwszego źródła światła
static float theta1 = 2.2;
static float phi1 = -0.4;
static float lightPosition1[] = {0.0, 0.0, 10.0, 1.0};

// Parametry drugiego źródła światła
static float theta2 = 0.9;
static float phi2 = 0.6;
static float lightPosition2[] = {0.0, 0.0, 10.0, 1.0};

// Funkcja rysująca osie układu współrzędnych
void drawAxes() {
    point3 x_min = {-5.0, 0.0, 0.0};
    point3 x_max = {5.0, 0.0, 0.0};
    // początek i koniec obrazu osi x

    point3 y_min = {0.0, -5.0, 0.0};
    point3 y_max = {0.0, 5.0, 0.0};
    // początek i koniec obrazu osi y

    point3 z_min = {0.0, 0.0, -5.0};
    point3 z_max = {0.0, 0.0, 5.0};
    //  początek i koniec obrazu osi y

    glColor3f(1.0f, 0.0f, 0.0f);  // kolor rysowania osi - czerwony
    glBegin(GL_LINES); // rysowanie osi x

    glVertex3fv(x_min);
    glVertex3fv(x_max);

    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);  // kolor rysowania - zielony
    glBegin(GL_LINES);  // rysowanie osi y

    glVertex3fv(y_min);
    glVertex3fv(y_max);

    glEnd();

    glColor3f(0.0f, 0.0f, 1.0f);  // kolor rysowania - niebieski
    glBegin(GL_LINES); // rysowanie osi z

    glVertex3fv(z_min);
    glVertex3fv(z_max);

    glEnd();

}

void drawWhiteTriangles() {
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - 1; j++) {
            glBegin(GL_TRIANGLE_FAN);

            glColor3f(1.0, 1.0, 1.0);

            glNormal3fv(ARRAY[i][j].normalized);
            glVertex3fv(ARRAY[i][j].point3);

            glNormal3fv(ARRAY[i + 1][j + 1].normalized);
            glVertex3fv(ARRAY[i + 1][j + 1].point3);

            glNormal3fv(ARRAY[i][j + 1].normalized);
            glVertex3fv(ARRAY[i][j + 1].point3);

            glNormal3fv(ARRAY[i + 1][j].normalized);
            glVertex3fv(ARRAY[i + 1][j].point3);

            glNormal3fv(ARRAY[i + 1][j + 1].normalized);
            glVertex3fv(ARRAY[i + 1][j + 1].point3);

            glEnd();
        }
    }
}

// Funkcja obsługująca rysowanie jajka
void Egg() {
    drawWhiteTriangles();
}

// Funkcja obsługująca rysowanie czajnika
void Teapot() {
    glColor3f(1.0, 1.0, 1.0);    // ustawienie koloru na biały
    glutSolidTeapot(4.0);
}

void renderScene() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // czyszczenie okna aktualnym kolorem czyszczącym

    glLoadIdentity();    // czyszczenie macierzy bieżącej

    // Obliczanie azymutu oraz elewacji dla obracanego obiektu
    if (rotationSwitch == 1) {
        if (status == 1) {      // jeśli lewy klawisz myszy wciśnięty
            objectRotation[0] += delta_x * pix2angle;
            objectRotation[1] += delta_y * pix2angle;

            if (objectRotation[0] > 360.0) objectRotation[0] -= 360.0;
            else if (objectRotation[0] < -360.0) objectRotation[0] += 360.0;
            if (objectRotation[1] > 360) objectRotation[1] -= 360.0;
            else if (objectRotation[1] < -360.0) objectRotation[1] += 360.0;

        } else if (status == 2) {       // jeśli prawy klawisz myszy wciśnięty
            viewerZ += delta_y / 10;
            if (viewerZ > 20) {
                viewerZ = 20;
            } else if (viewerZ < 2) {
                viewerZ = 2;
            }
        }
    }

    gluLookAt(0.0, 0.0, viewerZ, centerX, centerY, centerZ, 0.0, 1.0, 0.0);
    glRotatef(objectRotation[0], 0.0, 1.0, 0.0);
    glRotatef(objectRotation[1], 1.0, 0.0, 0.0);

    if (showAxes == 1) drawAxes();
    if (objectMode == 1) Egg();
    else Teapot();

    if (rotationSwitch == 2) {
        // Obliczanie kątów potrzebnych do zmiany położenia drugiego źródła światła
        if (status == 1) {                  // jeśli lewy klawisz myszy wciśnięty
            theta1 += (delta_x * pix2angle) / 100;    // modyfikacja kąta obrotu o kąt proporcjonalny
            phi1 += (delta_y * pix2angle) / 100;

            if (theta1 > 360.0) theta1 -= 360.0;
            else if (theta1 < -360.0) theta1 += 360.0;
            if (phi1 > 360) phi1 -= 360.0;
            else if (phi1 < -360.0) phi1 += 360.0;

        }
        // Obliczanie kątów potrzebnych do zmiany położenia drugiego źródła światła
        else if (status == 2) {        // jeśli prawy klawisz myszy wciśnięty
            theta2 += (delta_x * pix2angle) / 100;    // modyfikacja kąta obrotu o kąt proporcjonalny
            phi2 += (delta_y * pix2angle) / 100;

            if (theta2 > 360.0) theta2 -= 360.0;
            else if (theta2 < -360.0) theta2 += 360.0;
            if (phi2 > 360) phi2 -= 360.0;
            else if (phi2 < -360.0) phi2 += 360.0;
        }
    }
    lightPosition1[0] = radius * cos(theta1) * cos(phi1);
    lightPosition1[1] = radius * sin(phi1);
    lightPosition1[2] = radius * sin(theta1) * cos(phi1);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
    lightPosition2[0] = radius * cos(theta2) * cos(phi2);
    lightPosition2[1] = radius * sin(phi2);
    lightPosition2[2] = radius * sin(theta2) * cos(phi2);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);

    glutSwapBuffers();
}

// Inicjalizacja współrzędnych punktów potrzebnych do wyświetlenia jajka
void setVertices() {
    for (int i = 0; i < N; i++) {
        float u = (float) i / (N - 1);
        float uPow5 = pow(u, 5);
        float uPow4 = pow(u, 4);
        float uPow3 = pow(u, 3);
        float uPow2 = pow(u, 2);
        float x, y, z;
        float x_u, x_v, y_u, y_v, z_u, z_v;
        float vector_x, vector_y, vector_z;

        y = 160 * uPow4 - 320 * uPow3 + 160 * uPow2;

        for (int j = 0; j < N; j++) {
            float v = (float) j / (N - 1);

            x = (-90 * uPow5 + 225 * uPow4 - 270 * uPow3 + 180 * uPow2 - 45 * u) * cos(M_PI * v);
            z = (-90 * uPow5 + 225 * uPow4 - 270 * uPow3 + 180 * uPow2 - 45 * u) * sin(M_PI * v);

            ARRAY[i][j].point3[0] = x;
            ARRAY[i][j].point3[1] = y - 5;
            ARRAY[i][j].point3[2] = z;

            x_u = (-450 * uPow4 + 900 * uPow3 - 810 * uPow2 + 360 * u - 45) * cos(M_PI * v);
            x_v = M_PI * (90 * uPow5 - 225 * uPow4 + 270 * uPow3 - 180 * uPow2 + 45) * sin(M_PI * v);
            y_u = 640 * uPow3 - 960 * uPow2 + 320 * u;
            y_v = 0;
            z_u = (-450 * uPow4 + 900 * uPow3 - 810 * uPow2 + 360 * u - 45) * sin(M_PI * v);
            z_v = -M_PI * (90 * uPow5 - 225 * uPow4 + 270 * uPow3 - 180 * uPow2 + 45) * cos(M_PI * v);

            vector_x = y_u * z_v - z_u * y_v;
            vector_y = z_u * x_v - x_u * z_v;
            vector_z = x_u * y_v - y_u * x_v;

            float vector_len = sqrt(pow(vector_x, 2) + pow(vector_y, 2) + pow(vector_z, 2));
            if (vector_len == 0) {
                vector_len = 1;
            }

            vector_x = vector_x / vector_len;
            vector_y = vector_y / vector_len;
            vector_z = vector_z / vector_len;

            if (i == N - 1) {
                ARRAY[i][j].normalized[0] = ARRAY[0][j].normalized[0];
                ARRAY[i][j].normalized[1] = ARRAY[0][j].normalized[1];
                ARRAY[i][j].normalized[2] = ARRAY[0][j].normalized[2];
            } else {
                ARRAY[i][j].normalized[0] = vector_x;
                ARRAY[i][j].normalized[1] = vector_y;
                ARRAY[i][j].normalized[2] = vector_z;
            }

        }
    }
}

// Funkcja inicjalizuje tablicę 'ARRAY' oraz wywołuje funkcje inicjalizujące
// kolory i współrzędne punktów
void setArray() {
    cout << "Podaj liczbe N = ";
    cin >> N;
    VERTEXES **newArray = new VERTEXES *[N];
    for (int i = 0; i < N; i++) {
        newArray[i] = new VERTEXES[N];
    }
    ARRAY = newArray;
    setVertices();
}

// Funkcja wyświetlająca w konsoli informacje o opcjach programu
void initProgram() {
    cout << "Program z laboratorium 3 - modelowanie obiektow 3-D\n";
    setArray();
    cout << "Odpowiednie klawisze zmieniaja widok modelu:\n";
    cout << "   Wyswietlany obiekt:\n";
    cout << "      'f' - jajko (domyslne)\n";
    cout << "      'g' - czajnik\n";
    cout << "   Wybrany tryb obracania:\n";
    cout << "      '1' - obracanie obiektem (domyslne)\n";
    cout << "      '2' - obracanie zrodlami swiatla\n";
    cout << "   Wyswietlanie osi wspolrzednych:\n";
    cout << "      'a' - nie (domyslne)\n";
    cout << "      's' - tak\n";
    cout << "   Sterowanie myszka:\n";
    cout << "      'Przytrzymaj LPM' - obrot obiektu lub obrot pierwszego zrodla swiatla\n";
    cout << "      'Przytrzymaj PPM' - zoom obiektu lub obrot drugiego zrodla swiatla\n";
    cout << "   Operowanie programem:\n";
    cout << "      'esc' - wyjscie z programu\n";
}

// Funkcja obsługująca działanie programu za pomocą klawiszy klawiatury
void keys(unsigned char key, int x, int y) {
    if (key == '1') rotationSwitch = 1;
    if (key == '2') rotationSwitch = 2;
    if (key == 'a') showAxes = 0;
    if (key == 's') showAxes = 1;
    if (key == 'f') objectMode = 1;
    if (key == 'g') objectMode = 2;
    if (key == (char) 27) exit(0);

    renderScene(); // przerysowanie obrazu sceny
}

// Funkcja ma za zadanie utrzymanie stałych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokość i szerokość okna) sś
// przekazywane do funkcji za każdym razem, gdy zmieni się rozmiar okna.
void changeSize(GLsizei horizontal, GLsizei vertical) {

    pix2angle = 360.0 / (float) horizontal;  // przeliczenie pikseli na stopnie

    glMatrixMode(GL_PROJECTION);
    // Przełączenie macierzy bieżącej na macierz projekcji

    glLoadIdentity();
    // Czyszczenie macierzy bieżącej

    gluPerspective(70, 1.0, 1.0, 30.0);
    // Ustawienie parametrów dla rzutu perspektywicznego

    if (horizontal <= vertical) {
        glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);
    } else {
        glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
    }
    // Ustawienie wielkości okna widoku (viewport) w zależności
    // relacji pomiędzy wysokością i szerokością okna

    glMatrixMode(GL_MODELVIEW);
    // Przełączenie macierzy bieżącej na macierz widoku modelu

    glLoadIdentity();
    // Czyszczenie macierzy bieżącej

}

// Funkcja obsługuje wciśnięcie przycisku na myszce
void Mouse(int btn, int state, int x, int y) {
    if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        x_pos_old = x;         // przypisanie aktualnie odczytanej pozycji kursora jako pozycji poprzedniej
        y_pos_old = y;
        status = 1;          // wciśnięty został lewy klawisz myszy
    } else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        y_pos_old = y;
        status = 2;
    } else {
        status = 0;          // nie został wciśnięty żaden klawisz
    }
}

// Funkcja "monitoruje" położenie kursora myszy i ustawia wartości odpowiednich
// zmiennych globalnych
void Motion(GLsizei x, GLsizei y) {

    delta_x = x - x_pos_old;     // obliczenie różnicy położenia kursora myszy
    delta_y = y - y_pos_old;

    x_pos_old = x;            // podstawienie bieżącego położenia jako poprzednie
    y_pos_old = y;

    glutPostRedisplay();     // przerysowanie obrazu sceny
}

// Funkcja ustalająca stan renderowania
void myInit() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Kolor czyszczący (wypełnienia okna) ustawiono na czarny

/*************************************************************************************/
// Definicja materiału, z jakiego zrobiony jest obiekt

    GLfloat mat_ambient[] = {0.2, 0.2, 0.2, 0.2};
    // współczynniki ka =[kar,kag,kab] dla światła otoczenia

    GLfloat mat_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    // współczynniki kd =[kdr,kdg,kdb] światła rozproszonego

    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    // współczynniki ks =[ksr,ksg,ksb] dla światła odbitego

    GLfloat mat_shininess = {20.0};
    // współczynnik n opisujący połysk powierzchni

/*************************************************************************************/
// Definicja stałych odpowiadających za osłabienie światła dla obu źródeł

    GLfloat att_constant = {1.0};
    // składowa stała ds dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_linear = {0.05};
    // składowa liniowa dl dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

    GLfloat att_quadratic = {0.001};
    // składowa kwadratowa dq dla modelu zmian oświetlenia w funkcji
    // odległości od źródła

/*************************************************************************************/
// Definicja pierwszego źródła światła

    GLfloat light_ambient1[] = {0.1, 0.0, 0.0, 1.0};
    // składowe intensywności świecenia źródła światła otoczenia
    // Ia = [Iar,Iag,Iab]

    GLfloat light_diffuse1[] = {1.0, 0.0, 0.0, 1.0};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie dyfuzyjne Id = [Idr,Idg,Idb]

    GLfloat light_specular1[] = {0.85, 0.85, 0.85, 0.85};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie kierunkowe Is = [Isr,Isg,Isb]

/*************************************************************************************/
// Definicja drugiego źródła światła

    GLfloat light_ambient2[] = {0.0, 0.0, 0.1, 1.0};
    // składowe intensywności świecenia źródła światła otoczenia
    // Ia = [Iar,Iag,Iab]

    GLfloat light_diffuse2[] = {0.0, 0.0, 1.0, 1.0};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie dyfuzyjne Id = [Idr,Idg,Idb]

    GLfloat light_specular2[] = {0.85, 0.85, 0.85, 0.85};
    // składowe intensywności świecenia źródła światła powodującego
    // odbicie kierunkowe Is = [Isr,Isg,Isb]

/*************************************************************************************/
// Ustawienie parametrów materiału


    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


/*************************************************************************************/
// Ustawienie parametrów pierwszego źródła światła

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse1);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular1);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);

/*************************************************************************************/
// Ustawienie parametrów pierwszego źródła światła

    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient2);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse2);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular2);

    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, att_constant);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, att_linear);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, att_quadratic);


/*************************************************************************************/
// Ustawienie opcji systemu oświetlania sceny

    glShadeModel(GL_SMOOTH); // włączenie łagodnego cieniowania
    glEnable(GL_LIGHTING);   // włączenie systemu oświetlenia sceny
    glEnable(GL_LIGHT0);     // włączenie źródła o numerze 0
    glEnable(GL_LIGHT1);     // włączenie źródła o numerze 1
    glEnable(GL_DEPTH_TEST); // włączenie mechanizmu z-bufora

/*************************************************************************************/
    renderScene(); // przerysowanie obrazu sceny
}

// Główny punkt wejścia programu. Program działa w trybie konsoli
int main() {

    initProgram();

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(600, 600);

    glutCreateWindow("Oswietlenie scen 3-D");

    glutDisplayFunc(renderScene);
    // Określenie, że funkcja renderScene będzie funkcją zwrotną
    // (callback function).  Biedzie ona wywoływana za każdym razem,
    // gdy zajdzie potrzeba przerysowania okna

    glutReshapeFunc(changeSize);
    // Dla aktualnego okna ustala funkcję zwrotną odpowiedzialną
    // za zmiany rozmiaru okna

    glutKeyboardFunc(keys);

    glutMouseFunc(Mouse);
    // Ustala funkcję zwrotną odpowiedzialną za badanie stanu myszy

    glutMotionFunc(Motion);
    // Ustala funkcję zwrotną odpowiedzialną za badanie ruchu myszy

    myInit();
    // Funkcja myInit() (zdefiniowana powyżej) wykonuje wszelkie
    // inicjalizacje konieczne  przed przystąpieniem do renderowania

    glutMainLoop();
    // Funkcja uruchamia szkielet biblioteki GLUT

}
