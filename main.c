#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <math.h>

#define VERTICES_ARVORE 11

//tamanho da tela
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 800;

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

//estrutura de dados das faces
typedef struct face Face;
struct face
{
    Vertice v[VERTICES_ARVORE];
};

//vetor
typedef struct vetor2D Vetor2D;
struct vetor2D
{
    int x;
    int y;
    int z;
};

int criar_tela();                             //inicia o SDL e cria a tela
void fechar_tela();                          //fecha o SDL e fecha a tela
void trabalho();                            //função principal do programa
void criar_matriz_identidade(Matriz m);    //cria a matriz identidade para usar depois
void criar_matriz_identidade4(Matriz4 m);
void multiplicar_matrizes4(Matriz4 m1, Matriz4 m2);
void rotacao_matriz(Vertice *v);
void criar_arvore(Face faces[]);
void desenhar_arvore(Face faces[]);

int main(int argc, char *args[])
{
    setlocale(LC_ALL, "Portuguese");
    printf("\nTrabalho 2 de implementação de Computação Gráfica -\nAluna: Camilla Andrade Pereira-\n\n");
    //printf("\nAperte as teclas (em ordem das figuras)\n'a' \t 'b'\n'c' \t 'd'\npara responder.\n\n");
    //printf("\nO teste tem 5 perguntas, cada resposta certa vale 100 pontos.\nTotal máximo de 500 pontos.\n");
    trabalho();

    return 0;
}

void trabalho()
{
    Face arvore[2];

    if(!criar_tela())       //começa o SDL e cria a tela
        printf("\n\nFalha a inicializar!\n");
    else
    {
        int ok = 1;
        int sair = 0;       //flag principal do loop
        SDL_Event evento;   //event handler

        while(!sair)                            //enquanto a aplicação estiver rodando
        {
            while(SDL_PollEvent(&evento)!=0)    //lida com eventos na fila
            {
                if(evento.type==SDL_QUIT)       //usuário pede para sair
                    sair=1;
                else if(evento.type==SDL_KEYDOWN)
                {
                    switch(evento.key.keysym.sym)
                    {
                        case SDLK_a:

                            break;

                       /* case SDLK_b:
                        case SDLK_c:
                        case SDLK_d:
                            fase++;
                            ok = 1;
                            break;*/

                        case SDLK_ESCAPE:
                            sair = 1;
                            break;

                        default:
                            break;
                    }
                }
            }

            if(ok==1)
            {
                SDL_SetRenderDrawColor(renderizacao, 0xFF, 0xFF, 0xFF, 0x00);
                SDL_RenderClear(renderizacao);
                criar_arvore(arvore);

                for(int i=0;i<2;i++)
                {
                    for(int j=0;j<VERTICES_ARVORE;j++)
                    {
                        arvore[i].v[j].x *= 5;
                        arvore[i].v[j].y *= 5;
                        arvore[i].v[j].z *= 5;

                        arvore[i].v[j].x += 150;
                        arvore[i].v[j].y += 300;

                        rotacao_matriz(&arvore[i].v[j]);
                    }
                }

                ok = 0;
            }
            desenhar_arvore(arvore);
            SDL_RenderPresent(renderizacao);
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

void criar_matriz_identidade(Matriz m)
{
    //crio a matriz identidade para servir de base para as matrizes de transformações
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<3;j++)
        {
            if(i==j)
                m[i][j] = 1;
            else
                m[i][j] = 0;
        }
    }
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

    while((fscanf(arq, "%d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d; %d %d %d;\n", &faces[i].v[0].x, &faces[i].v[0].y, &faces[i].v[0].z, &faces[i].v[1].x, &faces[i].v[1].y, &faces[i].v[1].z, &faces[i].v[2].x, &faces[i].v[2].y, &faces[i].v[2].z, &faces[i].v[3].x, &faces[i].v[3].y, &faces[i].v[3].z, &faces[i].v[4].x, &faces[i].v[4].y, &faces[i].v[4].z, &faces[i].v[5].x, &faces[i].v[5].y, &faces[i].v[5].z, &faces[i].v[6].x, &faces[i].v[6].y, &faces[i].v[6].z, &faces[i].v[7].x, &faces[i].v[7].y, &faces[i].v[7].z, &faces[i].v[8].x, &faces[i].v[8].y, &faces[i].v[8].z, &faces[i].v[9].x, &faces[i].v[9].y, &faces[i].v[9].z, &faces[i].v[10].x, &faces[i].v[10].y, &faces[i].v[10].z) != EOF))
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
