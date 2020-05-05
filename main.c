#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <math.h>
#include "timer.h"

#define FPS             15
#define TICKS_POR_FRAME 1000/FPS
#define VERTICES_ARVORE 11
#define ANG_RAD(x) (x*M_PI)/180

#define LINHA 640
#define COLUNA 480

//tamanho da tela
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
float z_buffer[LINHA][COLUNA];

//variáveis globais
SDL_Window *janela = NULL;
SDL_Surface *superficie_tela = NULL;
SDL_Renderer *renderizacao = NULL;

//criando matrizes para as transformações
typedef double Matriz[3][3];
typedef double Matriz_vetor[3];
typedef double Matriz4[4][4];
typedef double Matriz4_vetor[4];

//estrutura de dados de vértices
typedef struct vertice Vertice;
struct vertice
{
    int x;
    int y;
    int z;
};

Vertice CurvasPontos[1000];
//estrutura de dados das faces
typedef struct face Face;
struct face
{
    Vertice v[VERTICES_ARVORE];
};

//arvore vai ser variável global
Face arvore[2];

//vetor
typedef struct vetor3D Vetor3D;
struct vetor3D
{
    float x;
    float y;
    float z;
};

//quartenions
typedef struct quartenion Quartenion;
struct quartenion
{
    float scalar;
    Vetor3D v;
};

int criar_tela();                             //inicia o SDL e cria a tela
void fechar_tela();                          //fecha o SDL e fecha a tela
void trabalho();                            //função principal do programa
void criar_matriz_identidade4(Matriz4 m);
void multiplicar_matrizes4(Matriz4 m1, Matriz4 m2);
void rotacao_matriz(Vertice *v);

void criar_arvore(Face faces[]);
void desenhar_arvore(Face faces[]);
void desenhar_linha_arvore(Vertice v1, Vertice v2, SDL_Color cor);
void desenhar_figura();
void arvore_loop(Vertice v1, Vertice v2, Vertice v3, SDL_Color cor);
void pintar_arvore(Face faces[]);
float calcular_z_depth(Vertice v1, Vertice v2, Vertice v3, int x, int y);
float area_triangulo(Vertice v1, Vertice v2, Vertice v3);
int max(int x1, int x2, int x3);
int min(int x1, int x2, int x3);
int equacao_reta_arvore(Vertice v1, Vertice v2, int x, int y);

void multiplicar_rotacao(Quartenion *q, Quartenion *p, Quartenion *q_inv);
void criar_quartenion_rotacao(Quartenion *q, double angulo, Vetor3D u);
void criar_quartenion_ponto(Quartenion *q, Vertice *v);
void quartenion_rotacao(Vertice *v, double angulo, Vetor3D u);
Quartenion multiplicao(Quartenion *q1, Quartenion *q2);

void criar_curva(Vertice p0, Vertice p1, Vertice p2, Vertice p3);
void desenhar_curva();
void transladar_arvore_ponto(Face faces[], Vertice p0);
void transladar_arvore(Face faces[], int pos);
Vetor3D max_vertice(int lados, Face *face);
Vetor3D min_vertice(int lados, Face *face);
Vetor3D centro_massa(Face faces[], int face);

float    vetor_norma(Vetor3D v);
float    vetor_interno(Vetor3D v, Vetor3D r);
Vetor3D  vetor_unitario(Vetor3D v);
Vetor3D  vetor_produto(Vetor3D v, Vetor3D r);


int main(int argc, char *args[])
{
    setlocale(LC_ALL, "Portuguese");
    printf("\nTrabalho 2 de implementação de Computação Gráfica\n\n");
    printf("\nAperte as teclas '1' para rotação com quatérnio e '2' para curvas.\n\n");
    trabalho();

    return 0;
}

void desenhar_figura()
{
    criar_arvore(arvore);

    for(int i=0;i<2;i++)
    {
        for(int j=0;j<VERTICES_ARVORE;j++)
        {
            arvore[i].v[j].x += 400;
            arvore[i].v[j].y += 500;
            rotacao_matriz(&arvore[i].v[j]);
        }
    }

    desenhar_arvore(arvore);
}

void desenhar_rotacao(double angulo, Vetor3D u)
{
    for(int i=0;i<2;i++)
        for(int j=0;j<VERTICES_ARVORE;j++)
            quartenion_rotacao(&arvore[i].v[j], angulo, u);

    desenhar_arvore(arvore);
}

void trabalho()
{
    Timer fpstempo, captempo;
    Vertice v1, v2, p0, p1, p2, p3;
    Vetor3D v, u;
    double angulo;
    double angulo_contador;
    int contador = 0;
    SDL_Color cor = {0, 0, 0, 0xFF};

    for(int i=0;i<SCREEN_WIDTH; i++)
        for(int j=0;j<SCREEN_HEIGHT;j++)
            z_buffer[i][j] = 10;

    start(&fpstempo);

    if(!criar_tela())       //começa o SDL e cria a tela
        printf("\n\nFalha a inicializar!\n");
    else
    {
        int ok = 0;
        int pos = 99;
        int opcao = 0;
        int sair = 0;       //flag principal do loop
        SDL_Event evento;   //event handler

        while(!sair)                            //enquanto a aplicação estiver rodando
        {
            start(&captempo);
            while(SDL_PollEvent(&evento)!=0)    //lida com eventos na fila
            {
                if(evento.type==SDL_QUIT)       //usuário pede para sair
                    sair=1;
                else if(evento.type==SDL_KEYDOWN)
                {
                    switch(evento.key.keysym.sym)
                    {
                        case SDLK_0:
                            opcao = 0;
                            break;

                        case SDLK_1:
                            opcao = 1;
                            break;

                        case SDLK_2:
                            opcao = 2;
                            ok = 0;
                            break;

                        case SDLK_ESCAPE:
                            sair = 1;
                            break;

                        default:
                            break;
                    }
                }
            }

            if(opcao==0)
            {
                SDL_SetRenderDrawColor(renderizacao, 0xFF, 0xFF, 0xFF, 0x00);
                SDL_RenderClear(renderizacao);

                desenhar_figura();

                SDL_RenderPresent(renderizacao);
            }
            else if(opcao==1)
            {
                printf("\n\nPor favor digite dois pontos para ser traçado uma reta - \n");
                printf("Ponto 1, x: ");
                scanf("%d", &v1.x);
                printf("Ponto 1, y: ");
                scanf("%d", &v1.y);
                printf("Ponto 1, z: ");
                scanf("%d", &v1.z);

                printf("\nPonto 2, x: ");
                scanf("%d", &v2.x);
                printf("Ponto 2, y: ");
                scanf("%d", &v2.y);
                printf("Ponto 2, z: ");
                scanf("%d", &v2.z);
                printf("%d %d %d %d %d %d", v1.x,v1.y,v1.z,v2.x,v2.y,v2.z);

                printf("\n\nPor favor digite um angulo - \n");
                scanf("%lf", &angulo);

                v.x = v2.x - v1.x;
                v.y = v2.y - v1.y;
                v.z = v2.z - v1.z;

                u = vetor_unitario(v);

                angulo_contador = angulo/50.0;
                contador = 0;
                opcao = 3;
            }
             else if(opcao==3)
            {
                SDL_SetRenderDrawColor(renderizacao, 0xFF, 0xFF, 0xFF, 0x00);
                SDL_RenderClear(renderizacao);

                desenhar_linha_arvore(v1,v2,cor);

                if(contador == 50)
                    opcao = 5;

                desenhar_rotacao(angulo_contador,u);
                contador++;
                desenhar_arvore(arvore);

                SDL_RenderPresent(renderizacao);
            }
            else if(opcao==2)
            {
                printf("\n\nPor favor digite quatro pontos para ser desenhado a curva - \n");
                printf("Ponto 1, x: ");
                scanf("%d", &p0.x);
                printf("Ponto 1, y: ");
                scanf("%d", &p0.y);
                printf("Ponto 1, z: ");
                scanf("%d", &p0.z);

                printf("\nPonto 2, x: ");
                scanf("%d", &p1.x);
                printf("Ponto 2, y: ");
                scanf("%d", &p1.y);
                printf("Ponto 2, z: ");
                scanf("%d", &p1.z);

                printf("\nPonto 3, x: ");
                scanf("%d", &p2.x);
                printf("Ponto 3, y: ");
                scanf("%d", &p2.y);
                printf("Ponto 3, z: ");
                scanf("%d", &p2.z);

                printf("\nPonto 4, x: ");
                scanf("%d", &p3.x);
                printf("Ponto 4, y: ");
                scanf("%d", &p3.y);
                printf("Ponto 4, z: ");
                scanf("%d", &p3.z);

                printf("P0[%d %d %d], P1[%d %d %d], P2[%d %d %d], P3[%d %d %d]",
                       p0.x,p0.y,p0.z,p1.x,p1.y,p1.z,p2.x,p2.y,p2.z,p3.x,p3.y,p3.z);

                criar_curva(p0, p1, p2, p3);

                opcao = 4;
            }
            else if(opcao==4)
            {
                SDL_SetRenderDrawColor(renderizacao, 0xFF, 0xFF, 0xFF, 0x00);
                SDL_RenderClear(renderizacao);

                desenhar_curva();
                if(ok == 0)
                {
                    transladar_arvore_ponto(arvore, p0);
                    ok = 1;
                }

                if(pos < 1000)
                    transladar_arvore(arvore, pos);
                else
                    opcao=5;

                pos += 100;
                desenhar_arvore(arvore);

                SDL_RenderPresent(renderizacao);
            }
            else if(opcao==5)
            {
                pintar_arvore(arvore);
                SDL_RenderPresent(renderizacao);
            }

        int frames_ticks = get_ticks(&captempo);

        if(frames_ticks<TICKS_POR_FRAME)
            SDL_Delay(TICKS_POR_FRAME - frames_ticks);
        }
    }
    fechar_tela();
}

int criar_tela()
{
    int funciona = 1; //flag para iniciar o SDL

    if(SDL_Init(SDL_INIT_VIDEO)<0) //inicializa o SDL e testa se funciona
    {
        printf("\n\nO SDL não pode inicializar! SDL_ERROR: %s\n", SDL_GetError());
        funciona = 0;
    }
    else
    {
        //cria a janela
        janela = SDL_CreateWindow("Trabalho 2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );

        if(janela==NULL)
        {
            printf("\n\nA janela não pode ser criada! SLD_ERROR: %s\n", SDL_GetError());
            funciona = 0;
        }
        else //pega a superfície da tela
        {
            renderizacao = SDL_CreateRenderer(janela,-1,SDL_RENDERER_ACCELERATED); //cria renderizacao para janela

            if(renderizacao==NULL )
			{
				printf("\n\nNão pode ser renderizado! SDL Error: %s\n",SDL_GetError());
				funciona = 0;
			}
			else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor(renderizacao, 0xFF, 0xFF, 0xFF, 0xFF );
                superficie_tela = SDL_GetWindowSurface(janela);
            }
        }
    }
    return funciona;

}

void fechar_tela()
{
    SDL_DestroyRenderer(renderizacao);
    SDL_DestroyWindow(janela);
    janela = NULL;
    renderizacao = NULL;

    SDL_Quit();
}

void transladar_arvore_ponto(Face faces[], Vertice p0)
{
    Vetor3D face1, face1_translador;
    face1 = centro_massa(faces,0);

    face1_translador.x =  p0.x - face1.x;
    face1_translador.y =  p0.y - face1.y;
    face1_translador.z =  p0.z - face1.z;

    for(int i=0;i<2;i++)
    {
        for(int j=0;j<VERTICES_ARVORE;j++)
        {
               arvore[i].v[j].x += face1_translador.x;
               arvore[i].v[j].y += face1_translador.y;
               arvore[i].v[j].z += face1_translador.z;
        }
    }
}

void transladar_arvore(Face faces[], int pos)
{
    Vetor3D face1_translador;

    face1_translador.x =  CurvasPontos[pos].x - CurvasPontos[pos-100].x;
    face1_translador.y =  CurvasPontos[pos].y - CurvasPontos[pos-100].y;
    face1_translador.z =  CurvasPontos[pos].z - CurvasPontos[pos-100].z;

    for(int i=0;i<2;i++)
    {
        for(int j=0;j<VERTICES_ARVORE;j++)
        {
               arvore[i].v[j].x += face1_translador.x;
               arvore[i].v[j].y += face1_translador.y;
               arvore[i].v[j].z += face1_translador.z;

        }
    }
}

Vetor3D centro_massa(Face faces[], int face)
{
    Vetor3D g, v1, v2;
    v1 = max_vertice(VERTICES_ARVORE, &faces[face]);
    v2 = min_vertice(VERTICES_ARVORE, &faces[face]);
    g.x = (v1.x + v2.x)/2;
    g.y = (v1.y + v2.y)/2;
    g.z = (v1.z + v2.z)/2;

    return g;
}

Vetor3D  max_vertice(int lados, Face *face)
{
    Vetor3D max;
    max.x = -10000;
    max.y = -10000;
    max.z = -10000;

    for(int i=0; i<lados; i++)
    {
        if(face->v[i].x >= max.x)
            max.x = face->v[i].x;

        if(face->v[i].y >= max.y)
            max.y = face->v[i].y;

        if(face->v[i].z >= max.z)
            max.z = face->v[i].z;
    }

    return max;
}

Vetor3D min_vertice(int lados, Face *face)
{
    Vetor3D min;
    min.x = 10000;
    min.y = 10000;
    min.z = 10000;

    for(int i=0; i<lados; i++)
    {
        if(face->v[i].x <= min.x)
            min.x = face->v[i].x;

        if(face->v[i].y <= min.y)
            min.y = face->v[i].y;

        if(face->v[i].z <= min.z)
            min.z = face->v[i].z;
    }

    return min;
}


void criar_curva(Vertice p0, Vertice p1, Vertice p2, Vertice p3)
{
    int i = 0;
    for(double t = 0; t <= 1; t += .001)
    {
      CurvasPontos[i].x = pow(1-t,3)*p0.x + 3*t*pow(1-t,2)*p1.x + 3*t*t*(1-t)*p2.x + pow(t,3)*p3.x;
      CurvasPontos[i].y = pow(1-t,3)*p0.y + 3*t*pow(1-t,2)*p1.y + 3*t*t*(1-t)*p2.y + pow(t,3)*p3.y;
      i++;
    }
}

void desenhar_curva()
{
    SDL_SetRenderDrawColor(renderizacao,0,255,0,255);

    for(int i = 0; i < 1000; i++)
        SDL_RenderDrawPoint(renderizacao, CurvasPontos[i].x, CurvasPontos[i].y);
}

void criar_quartenion_rotacao(Quartenion *q, double angulo, Vetor3D u)
{
    double theta = sin(ANG_RAD(angulo)/2);

    q->scalar = cos(ANG_RAD(angulo)/2);
    q->v.x = u.x * theta;
    q->v.y = u.y * theta;
    q->v.z = u.z * theta;
}

void criar_quartenion_ponto(Quartenion *q, Vertice *v)
{
    q->scalar = 0;
    q->v.x = v->x;
    q->v.y = v->y;
    q->v.z = v->z;
}

void quartenion_rotacao(Vertice *v, double angulo, Vetor3D u)
{
    Quartenion q, p, q_inv;
    criar_quartenion_rotacao(&q, angulo, u);
    criar_quartenion_rotacao(&q_inv, angulo, u);
    q_inv.v.x *= -1;
    q_inv.v.y *= -1;
    q_inv.v.z *= -1;
    criar_quartenion_ponto(&p, v);

    multiplicar_rotacao(&q, &p, &q_inv);

    v->x = p.v.x;
    v->y = p.v.y;
    v->z = p.v.z;
}

void multiplicar_rotacao(Quartenion *q, Quartenion *p, Quartenion *q_inv)
{
    Quartenion p_linha;
    Vetor3D v, r, vxr, vxvxr;
    float scalar = q->scalar;

    v.x = q->v.x;
    v.y = q->v.y;
    v.z = q->v.z;

    r.x = p->v.x;
    r.y = p->v.y;
    r.z = p->v.z;

    vxr = vetor_produto(v, r);
    vxvxr = vetor_produto(v, vxr);

    p_linha.v.x = r.x + 2*scalar*vxr.x + 2*vxvxr.x;
    p_linha.v.y = r.y + 2*scalar*vxr.y + 2*vxvxr.y;
    p_linha.v.z = r.z + 2*scalar*vxr.z + 2*vxvxr.z;

    p->v.x = p_linha.v.x;
    p->v.y = p_linha.v.y;
    p->v.z = p_linha.v.z;

}

Quartenion multiplicao(Quartenion *p, Quartenion *q)
{
    Quartenion q3;

    q3.scalar = p->scalar * q->scalar - (p->v.x*q->v.x + p->v.y*q->v.y + p->v.z*q->v.z);
    q3.v.x = p->scalar*q->v.x + q->scalar*p->v.x + (p->v.y*q->v.z - p->v.z*q->v.y);
    q3.v.y = p->scalar*q->v.y + q->scalar*p->v.y + (p->v.z*q->v.x - p->v.x*q->v.z);
    q3.v.z = p->scalar*q->v.z + q->scalar*p->v.z + (p->v.x*q->v.y - p->v.y*q->v.x);

    return q3;
}

float vetor_norma(Vetor3D v)
{
    return sqrt(vetor_interno(v,v));
}

Vetor3D vetor_unitario(Vetor3D v)
{
    Vetor3D u;
    float norma = vetor_norma(v);
    u.x = v.x/norma;
    u.y = v.y/norma;
    u.z = v.z/norma;

    return u;
}

float vetor_interno(Vetor3D v, Vetor3D r)
{
    return v.x*r.x + v.y*r.y + v.z*r.z;
}

Vetor3D  vetor_produto(Vetor3D v, Vetor3D r)
{
    Vetor3D vxr;
    vxr.x = v.y*r.z - v.z*r.y;
    vxr.y = v.z*r.x - v.x*r.z;
    vxr.z = v.x*r.y - v.y*r.x;

    return vxr;
}


void multiplicar_matrizes(Matriz m1, Matriz_vetor m2)
{
    //função para multiplicar a matriz (de transformação) pela matriz coluna dos vértices do polígono
    Matriz_vetor temp;

    for(int i=0;i<3;i++)
            temp[i] = m1[i][0]*m2[0] + m1[i][1]*m2[1] + m1[i][2]*m2[2];

    for(int i=0;i<3;i++)
            m2[i] = temp[i];

}

void criar_matriz_identidade4(Matriz4 m)
{
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            if (i==j)
                m[i][j] = 1;
            else
                m[i][j] = 0;
        }
    }
}

void multiplicar_matrizes4(Matriz4 m1, Matriz4 m2)
{
    Matriz4 temp;

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            temp[i][j] = m1[i][0]*m2[0][j] + m1[i][1]*m2[1][j] + m1[i][2]*m2[2][j] + m1[i][3]*m2[3][j];

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            m1[i][j] = temp[i][j];
}

void multiplicar_matriz4_vetor4(Matriz4 m, Matriz4_vetor v)
{
    Matriz4_vetor temp;

    for(int i=0;i<4;i++)
        temp[i] = m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2] + m[i][3]*v[3];

    for(int i=0;i<4;i++)
        v[i] = temp[i];
}

void rotacao_matriz(Vertice *v)
{
    Matriz4 m1;
    criar_matriz_identidade4(m1);

    Matriz4 m2;
    criar_matriz_identidade4(m2);

    m1[0][0] = cos(M_PI_4);
    m1[0][2] = -sin(M_PI_4);
    m1[2][0] = sin(M_PI_4);
    m1[2][2] = cos(M_PI_4);

    m2[1][1] = cos(35.26 * (M_PI/180));
    m2[1][2] = -sin(35.26 * (M_PI/180));
    m2[2][1] = sin(35.26 * (M_PI/180));
    m2[2][2] = cos(35.26 * (M_PI/180));

    Matriz4 m3;
    criar_matriz_identidade4(m3);
    m3[2][2] = 0;

    Matriz4_vetor ve;
    ve[0] = v->x;
    ve[1] = v->y;
    ve[2] = v->z;
    ve[3] = 1;

    multiplicar_matrizes4(m2,m1);
    multiplicar_matrizes4(m3,m2);
    multiplicar_matriz4_vetor4(m3,ve);

    v->x = ve[0];
    v->y = ve[1];
    v->z = ve[2];
}

void criar_arvore(Face faces[])
{
    int i = 0;
    FILE *arq;
    arq = fopen("arvore.txt","r");

    if(arq==NULL)
    {
        printf("\nErro ao abrir o arquivo dos polígonos.\n");
        exit(1);
    }

    while((fscanf(arq, "%d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d;\n",
                &faces[i].v[0].x, &faces[i].v[0].y, &faces[i].v[0].z, &faces[i].v[1].x, &faces[i].v[1].y, &faces[i].v[1].z, &faces[i].v[2].x, &faces[i].v[2].y, &faces[i].v[2].z,
                &faces[i].v[3].x, &faces[i].v[3].y, &faces[i].v[3].z, &faces[i].v[4].x, &faces[i].v[4].y, &faces[i].v[4].z, &faces[i].v[5].x, &faces[i].v[5].y, &faces[i].v[5].z,
                &faces[i].v[6].x, &faces[i].v[6].y, &faces[i].v[6].z, &faces[i].v[7].x, &faces[i].v[7].y, &faces[i].v[7].z, &faces[i].v[8].x, &faces[i].v[8].y, &faces[i].v[8].z,
                &faces[i].v[9].x, &faces[i].v[9].y, &faces[i].v[9].z, &faces[i].v[10].x, &faces[i].v[10].y, &faces[i].v[10].z) != EOF))
    {
        i++;
    }

    fclose(arq);
}

void desenhar_arvore(Face faces[])
{
    SDL_Color cor = {0, 0, 200, 0xFF};

    for(int i=0;i<2;i++)
    {
        for(int j=0;j<VERTICES_ARVORE;j++)
        {
            if (j+1==VERTICES_ARVORE)
                desenhar_linha_arvore(faces[i].v[j], faces[i].v[0], cor);
            else
                desenhar_linha_arvore(faces[i].v[j], faces[i].v[j+1], cor);
        }
    }

    for(int i=0;i<VERTICES_ARVORE;i++)
        desenhar_linha_arvore(faces[0].v[i], faces[1].v[i], cor);
}

void desenhar_linha_arvore(Vertice v1, Vertice v2, SDL_Color cor)
{
    SDL_SetRenderDrawColor(renderizacao,cor.r,cor.b,cor.g,cor.a);
    SDL_RenderDrawLine(renderizacao,v1.x,v1.y,v2.x,v2.y);
}

void pintar_arvore(Face faces[])
{
    SDL_Color cor2 = {50, 250, 150, 0};
    arvore_loop(faces[1].v[10],faces[1].v[0],faces[1].v[1],cor2);
    arvore_loop(faces[1].v[10],faces[1].v[1],faces[1].v[8],cor2);
    arvore_loop(faces[1].v[10],faces[1].v[8],faces[1].v[9],cor2);
    arvore_loop(faces[1].v[1],faces[1].v[2],faces[1].v[3],cor2);
    arvore_loop(faces[1].v[1],faces[1].v[3],faces[1].v[6],cor2);
    arvore_loop(faces[1].v[1],faces[1].v[6],faces[1].v[7],cor2);
    arvore_loop(faces[1].v[1],faces[1].v[7],faces[1].v[8],cor2);
    arvore_loop(faces[1].v[3],faces[1].v[4],faces[1].v[5],cor2);
    arvore_loop(faces[1].v[3],faces[1].v[5],faces[1].v[6],cor2);

    SDL_Color cor3 = {93, 255, 177, 0};
    arvore_loop(faces[0].v[10],faces[0].v[0],faces[1].v[0],cor3);
    arvore_loop(faces[0].v[10],faces[1].v[0],faces[1].v[10],cor3);
    arvore_loop(faces[0].v[0],faces[1].v[0],faces[1].v[1],cor3);
    arvore_loop(faces[0].v[0],faces[1].v[1],faces[0].v[1],cor3);
    arvore_loop(faces[0].v[1],faces[0].v[2],faces[1].v[2],cor3);
    arvore_loop(faces[0].v[1],faces[1].v[2],faces[1].v[1],cor3);
    arvore_loop(faces[0].v[3],faces[0].v[2],faces[1].v[2],cor3);
    arvore_loop(faces[0].v[3],faces[1].v[2],faces[1].v[3],cor3);
    arvore_loop(faces[0].v[3],faces[0].v[4],faces[1].v[4],cor3);
    arvore_loop(faces[0].v[3],faces[1].v[4],faces[1].v[3],cor3);
    arvore_loop(faces[0].v[5],faces[0].v[4],faces[1].v[4],cor3);
    arvore_loop(faces[0].v[5],faces[1].v[4],faces[1].v[5],cor3);
    arvore_loop(faces[0].v[6],faces[0].v[5],faces[1].v[5],cor3);
    arvore_loop(faces[0].v[6],faces[1].v[5],faces[1].v[6],cor3);
    arvore_loop(faces[0].v[7],faces[0].v[6],faces[1].v[6],cor3);
    arvore_loop(faces[0].v[7],faces[1].v[6],faces[1].v[7],cor3);
    arvore_loop(faces[0].v[8],faces[0].v[7],faces[1].v[7],cor3);
    arvore_loop(faces[0].v[8],faces[1].v[7],faces[1].v[8],cor3);
    arvore_loop(faces[0].v[9],faces[0].v[8],faces[1].v[8],cor3);
    arvore_loop(faces[0].v[9],faces[1].v[8],faces[1].v[9],cor3);
    arvore_loop(faces[0].v[10],faces[0].v[9],faces[1].v[9],cor3);
    arvore_loop(faces[0].v[10],faces[1].v[9],faces[1].v[10],cor3);

    SDL_Color cor = {50, 250, 150, 0};
    arvore_loop(faces[0].v[10],faces[0].v[0],faces[0].v[1],cor);
    arvore_loop(faces[0].v[10],faces[0].v[1],faces[0].v[8],cor);
    arvore_loop(faces[0].v[10],faces[0].v[8],faces[0].v[9],cor);
    arvore_loop(faces[0].v[1],faces[0].v[2],faces[0].v[3],cor);
    arvore_loop(faces[0].v[1],faces[0].v[3],faces[0].v[6],cor);
    arvore_loop(faces[0].v[1],faces[0].v[6],faces[0].v[7],cor);
    arvore_loop(faces[0].v[1],faces[0].v[7],faces[0].v[8],cor);
    arvore_loop(faces[0].v[3],faces[0].v[4],faces[0].v[5],cor);
    arvore_loop(faces[0].v[3],faces[0].v[5],faces[0].v[6],cor);

}

void arvore_loop(Vertice v1, Vertice v2, Vertice v3, SDL_Color cor)
{
    float z_depth;

    int xmin = min(v1.x,v2.x,v3.x);
    int xmax = max(v1.x,v2.x,v3.x);

    int ymin = min(v1.y,v2.y,v3.y);
    int ymax = max(v1.y,v2.y,v3.y);

    for(int i=xmin;i<=xmax;i++)
    {
        for(int j=ymin;j<=ymax;j++)
        {
            if(equacao_reta_arvore(v1,v2,i,j) <= 0 && equacao_reta_arvore(v2,v3,i,j) <= 0
                && equacao_reta_arvore(v3,v1, i,j) <= 0)
            {
                z_depth = calcular_z_depth(v1,v2,v3,i,j);

                if(z_depth < z_buffer[i][j])
                {
                    z_buffer[i][j] = z_depth;
                    SDL_SetRenderDrawColor(renderizacao,cor.r,cor.g,cor.b,cor.a);
                    SDL_RenderDrawPoint(renderizacao, i, j);
                }
            }
        }
    }
}

float calcular_z_depth(Vertice v1, Vertice v2, Vertice v3, int x, int y)
{
    Vertice v4;
    v4.x = x;
    v4.y = y;
    v4.z = 0;

    float AreaABC = area_triangulo(v1,v2,v3);

    float u = area_triangulo(v3,v1,v4)/AreaABC;
    float v = area_triangulo(v1,v2,v4)/AreaABC;
    float w = area_triangulo(v2,v3,v4)/AreaABC;
    float z_depth = (u/v1.z) + (v/v2.z) + (w/v3.z);

    return 1/z_depth;
}

float area_triangulo(Vertice v1, Vertice v2, Vertice v3)
{
    Vetor3D A, B, C;

    A.x = v2.x - v1.x;
    A.y = v2.y - v1.y;
    A.z = v2.z - v1.z;

    B.x = v3.x - v1.x;
    B.y = v3.y - v1.y;
    B.z = v3.z - v1.z;

    C.x = A.y*B.z - A.z*B.y;
    C.y = A.z*B.x - A.x*B.z;
    C.z = A.x*B.y - A.y*B.x;

    float area = (sqrt(C.x*C.x + C.y*C.y + C.z*C.z)/2.0);

    return area;
}

int equacao_reta_arvore(Vertice v1, Vertice v2, int x, int y)
{
    //o "produto vetorial" de R2 é usado para equação da reta que liga dois vértices
    //de um lado do poligono, é "basicamente" a equação de um dos lados do poligono
    int A = v1.y - v2.y;
    int B = v2.x - v1.x;
    int C = v1.x*v2.y - v1.y*v2.x;

    return A*x + B*y + C;
}

int max(int x1, int x2, int x3)
{

    if((x1>=x2) && (x1>=x3))
        return x1;
    else if((x2>=x3) && (x2>=x1))
        return x2;
    else
        return x3;
}

int min(int x1, int x2, int x3)
{

    if((x1<=x2) && (x1<=x3))
        return x1;
    else if((x2<=x3) && (x2<=x1))
        return x2;
    else
        return x3;
}
