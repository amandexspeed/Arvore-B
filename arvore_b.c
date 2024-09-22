#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "arvore_b.h"
#include "no.h"
#include "cliente.h"
#include "metadados.h"
#include "lista_nos.h"

// Retorna 0 se nó não for folha e 1 se for.
int eh_folha(No *no);
// Busca binária em um nó
int busca_binaria_no(No *no, int cod_cli, int *encontrou);
// Caso nó esteja na folha, remove
void removeNoFolha(FILE *arqDados, No *no, int i, int pont);
// Caso nó intermediário, remove e substitui pelo maior sucessor imediato
void substituiPorSucessor(FILE *arq, No *no, int index);
// Redistribui os nós se necessário
void redistribuirNos(FILE *arqDados, No *noExc, No *noIrmao, No *noPai, int idxPai, int pont);
// Ordena vetor de clientes
void insertionSort(Cliente **aux, int tamanho);
void balancearArvore(FILE *arqDados, No *noExc, int pont);
void concatenarNos(FILE *arq, No *no, No *irmao, No *pai, int idxPai, int pont);
int encontrarIndicePai(No *pai, int pont);

int eh_folha(No *no)
{
  for (int i = 0; i <= no->m; i++)
    if (no->p[i] != -1)
      return 0;
  return 1;
}

int busca_binaria_no(No *no, int cod_cli, int *encontrou)
{
  int esquerda = 0;
  int direita = no->m - 1;
  int meio;

  while (esquerda <= direita)
  {
    meio = (esquerda + direita) / 2;
    int chave_meio = no->clientes[meio]->cod_cliente;

    if (cod_cli == chave_meio)
    {
      *encontrou = 1;
      return meio; // chave encontrada no índice do meio
    }
    else if (cod_cli < chave_meio)
      direita = meio - 1;
    else
      esquerda = meio + 1;
  }
  *encontrou = 0;
  return esquerda; // chave não encontrada, retorna onde deve ser inserido
}

int busca(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados, int *pont, int *encontrou)
{
  Metadados *m_dados = le_arq_metadados(nome_arquivo_metadados);
  FILE *arq_dados = fopen(nome_arquivo_dados, "rb");
  if (!arq_dados)
  {
    printf("Erro ao abrir arquivo de dados.\n");
    return -1;
  }
  int pont_atual = m_dados->pont_raiz;
  *encontrou = 0;

  if (pont_atual == -1)
  {
    // árvore é vazia
    *pont = -1;
    fclose(arq_dados);
    return 0; // posição 0 em um nó vazio
  }

  while (pont_atual != -1)
  {
    // Seek para a posição do nó atual
    if (fseek(arq_dados, pont_atual, SEEK_SET) != 0)
    {
      printf("Erro ao posicionar no arquivo de dados.\n");
      fclose(arq_dados);
      return -1;
    }

    No *no_atual = le_no(arq_dados);
    if (no_atual == NULL)
    {
      printf("Erro ao ler o noh do arquivo de dados.\n");
      fclose(arq_dados);
      return -1;
    }

    int i = busca_binaria_no(no_atual, cod_cli, encontrou);
    if (*encontrou)
    {
      // Chave encontrada
      *pont = pont_atual;
      libera_no(no_atual);
      fclose(arq_dados);
      return i; // retorna posição onde a chave foi encontrada
    }
    else if (eh_folha(no_atual))
    {
      // Chave não encontrada e não é possível descer nível
      *pont = pont_atual;
      libera_no(no_atual);
      fclose(arq_dados);
      return i; // retorna posição onde a chave deve ser inserida
    }
    else
    {
      // Move-se para o nó filho
      if (i < 0 || i > no_atual->m)
      {
        printf("Erro: indice fora do intervalo.\n");
        libera_no(no_atual);
        fclose(arq_dados);
        return -1;
      }
      pont_atual = no_atual->p[i];
      libera_no(no_atual);
    }
  }
  // chave não encontrada, deve ser inserido na raiz
  *pont = -1;
  *encontrou = 0;
  fclose(arq_dados);
  return 0;
}


int esta_cheio(No * no){
  return no->m == 2*D;
}

void ordenar_no(No *no) {
  Cliente *aux;
  int aux_p;
  for (int i = 0; i < no->m; i++) {
    for (int j = i + 1; j < no->m; j++) {
      if (no->clientes[j]->cod_cliente < no->clientes[i]->cod_cliente) {
        aux = no->clientes[j];
        no->clientes[j] = no->clientes[i];
        no->clientes[i] = aux;

        // Manter os ponteiros sincronizados
        aux_p = no->p[j + 1];
        no->p[j + 1] = no->p[i + 1];
        no->p[i + 1] = aux_p;
      }
    }
  }
}

/* Divide nó */
/* 
  -> Função responsável por dividir o nó
  -> o nó P recebe fica com uma posição a mais
  -> depois é ordenado
  -> todas as chaves depois do meio são passadas para o nó Q
  -> a chave do meio é removida do nó P e retornada, pois será promovida ao nó pai
*/
Cliente* divide_no(No *p, No *q, int meio_p, Cliente *novo_cliente) {
  p->clientes[p->m] = novo_cliente;  // o no atual recebe um nó além da sua capacidade temporariamente
  p->p[p->m+1] = -1; // define o endereço do ponteiro da direita da chave que foi inserida como -1
  p->m++;
  ordenar_no(p);

  Cliente *chave_promovida = p->clientes[meio_p];  // Chave a ser promovida (meio é equivalente a D+1)
  int i;
  for (i = meio_p + 1; i < p->m; i++) {
    q->clientes[i - (meio_p + 1)] = p->clientes[i];
    q->p[i - (meio_p + 1)] = p->p[i];

    // remove posições do nó p
    p->clientes[i] = NULL;
    p->p[i] = -1;
  }
  
  // define o tamanho do novo nó
  q->m = D;

  // remove a chave que foi promovida
  p->clientes[meio_p] = NULL;
  p->p[meio_p+1] = -1;
  
  
  p->m = D; // Modifica o tamanho da página atual

  return chave_promovida;
}

/* Insere na raíz cheia */
/* 
  -> Função responsável por inserir dado em uma raíz que está cheia
  -> é executada depois que é feita a partição
  -> nó P fica com as D primeiras chaves
  -> nó Q fica com as D últimas
  -> a chave D+1 é promovida, e é inserida na nova raíz
  -> a nova raíz é um nó que guarda apenas a chave promovida
  -> retorna o último valor inserido na variável "posicao_livre"
*/
int insere_em_raiz_cheia(No *p, No *q, Metadados *m_dados, int posicao_livre, FILE *arq_dados, char *nome_arquivo_metadados, Cliente *chave_promovida, Cliente *chave_promovida_anterior, int end_direita) {
  No *nova_raiz = no(1,-1);
  nova_raiz->clientes[0] = chave_promovida;
  
  if(chave_promovida_anterior != NULL && chave_promovida_anterior->cod_cliente == chave_promovida->cod_cliente) {
    q->p[0] = end_direita;
  }

  m_dados = le_arq_metadados(nome_arquivo_metadados);
  posicao_livre = m_dados->pont_prox_no_livre; // pega a proxima posição livre pra inserir o nó q
  m_dados->pont_prox_no_livre+=tamanho_no(); // calcular a próxima posição livre, posteriormente será a nova raíz

  // insere os ponteiros que vão apontar pro filho esquerdo e filho direito da nova raíz
  nova_raiz->p[0] = m_dados->pont_raiz;
  nova_raiz->p[1] = posicao_livre;
  
  // nó p e q no campo pont_pai apontando para a nova raíz
  p->pont_pai = m_dados->pont_prox_no_livre;
  q->pont_pai = m_dados->pont_prox_no_livre;

  // salva nó da esquerda
  fseek(arq_dados, m_dados->pont_raiz, SEEK_SET);
  salva_no(p, arq_dados);

  // salva nó da direita
  fseek(arq_dados, posicao_livre, SEEK_SET);
  salva_no(q, arq_dados);

  // salva nó raiz
  fseek(arq_dados, m_dados->pont_prox_no_livre, SEEK_SET);
  salva_no(nova_raiz, arq_dados);

  m_dados->pont_raiz = m_dados->pont_prox_no_livre; // insere o novo endereço da nova raíz
  m_dados->pont_prox_no_livre+=tamanho_no(); // calcular o próximo nó livre
  salva_arq_metadados(nome_arquivo_metadados,m_dados);

  fclose(arq_dados);

  return posicao_livre;
}


/* Particionamento */
/* 
  -> Essa função é responsável por particionar o nó
  -> Ela é recursiva, isso serve para casos em que os nós pais estão cheios, então são particionados também
  -> Retorna o ponteiro que aponta para onde foi inserido o novo nó
*/
int particionamento(No *p, int pont_chave, Cliente *novo_cliente, char *nome_arquivo_metadados, FILE *arq_dados, Cliente *chave_promovida, int *end_direita) {
  No *q = no(0, 0);
  int pont_chave_no_atual = pont_chave;  // Guarda chave do nó atual
  int meio = ((p->m+1) / 2);     // Posição do meio do nó
  Metadados *m_dados;
  int posicao_livre;


  Cliente *chave_promovida_anterior = chave_promovida; // guarda a chave promovida anteriormente
  chave_promovida = divide_no(p, q, meio, novo_cliente); // divide e recebe a nova chave promocida
  

  if(p->pont_pai != -1) {
    // Atualizar o ponteiro do novo nó para um endereço correto
    m_dados = le_arq_metadados(nome_arquivo_metadados);
    posicao_livre = m_dados->pont_prox_no_livre; 

    m_dados->pont_prox_no_livre+=tamanho_no();
    *end_direita = posicao_livre;
    
    salva_arq_metadados(nome_arquivo_metadados,m_dados);
    
    // Busca o pai da chave promovida
    fseek(arq_dados, p->pont_pai, SEEK_SET);
    No *no_pai = le_no(arq_dados);
    
    // Se o pont_pai não é raíz e não está cheio
    if (!esta_cheio(no_pai))
    {
      // Inserir a chave promovida no nó pai
      no_pai->clientes[no_pai->m] = chave_promovida;
      no_pai->p[no_pai->m+1] = posicao_livre;  // Atualizar ponteiro para o novo nó
      no_pai->m++;
      ordenar_no(no_pai);
      // Salvar nó pai atualizado
      fseek(arq_dados, p->pont_pai, SEEK_SET);
      salva_no(no_pai, arq_dados);

      // Salvar nó atual atualizado
      fseek(arq_dados, pont_chave_no_atual, SEEK_SET);
      salva_no(p, arq_dados);

      // Salvar novo nó
      fseek(arq_dados, posicao_livre, SEEK_SET);
      salva_no(q, arq_dados);
      
      fclose(arq_dados);
      
      return pont_chave;
    }

    // Se o pont_pai não é raíz, mas está cheio
    // Salvar nó atual atualizado
    fseek(arq_dados, pont_chave_no_atual, SEEK_SET);
    salva_no(p, arq_dados);

    // Salvar novo nó
    fseek(arq_dados, posicao_livre, SEEK_SET);
    salva_no(q, arq_dados);

    particionamento(no_pai, pont_chave, chave_promovida, nome_arquivo_metadados, arq_dados, chave_promovida, end_direita);
    return pont_chave;
  }

  // caso o nó seja raíz cheia
  posicao_livre = insere_em_raiz_cheia(p, q, m_dados, posicao_livre, arq_dados, nome_arquivo_metadados, chave_promovida, chave_promovida_anterior, *end_direita);
}

int insere(int cod_cli, char *nome_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados) {
  int pontChave;
  int encontrou = 0;
  Cliente *novo_cliente = cliente(cod_cli, nome_cli);

  busca(cod_cli, nome_arquivo_metadados, nome_arquivo_dados, &pontChave, &encontrou);

  if (encontrou) {  // Caso o dado já exista na árvore
    return -1;
  }

  FILE *arqDados = fopen(nome_arquivo_dados, "rb+");
  if (arqDados == NULL) {
    return -1;
  }

  fseek(arqDados, pontChave, SEEK_SET);
  No *no_atual = le_no(arqDados);
  
  // Executa
  if (!esta_cheio(no_atual)) {
    no_atual->clientes[no_atual->m] = novo_cliente;
    no_atual->m++;
    ordenar_no(no_atual);
    fseek(arqDados, pontChave, SEEK_SET);
    salva_no(no_atual, arqDados);
    fclose(arqDados);
    return pontChave;
  }
  Cliente *chave_promovida = NULL;
  int end_direita = -1;
  particionamento(no_atual, pontChave, novo_cliente, nome_arquivo_metadados, arqDados, chave_promovida, &end_direita);
 
  return pontChave;
}

void removeNoFolha(FILE *arqDados, No *no, int i, int pont)
{
  // Caso 1: A chave está em um nó folha, remove diretamente
  for (int j = i; j < no->m - 1; j++)
  {
    no->clientes[j] = no->clientes[j + 1];
    no->p[j] = no->p[j + 1];
  }
  no->clientes[no->m - 1] = NULL; // Remove a última chave
  no->m--;                        // Decrementa o número de chaves no nó
  fseek(arqDados, pont, SEEK_SET);
  salva_no(no, arqDados);
}

void substituiPorSucessor(FILE *arq, No *no, int index)
{
  int sucessor_pont = no->p[index + 1];
  No *no_sucessor = NULL;

  do
  {
    fseek(arq, sucessor_pont, SEEK_SET);
    no_sucessor = le_no(arq);
    if (!eh_folha(no_sucessor))
    {
      sucessor_pont = no_sucessor->p[0];
    }
  } while (!eh_folha(no_sucessor));

  no->clientes[index] = no_sucessor->clientes[0];
  removeNoFolha(arq, no_sucessor, 0, sucessor_pont);

  libera_no(no_sucessor);
}

void insertionSort(Cliente **aux, int tamanho)
{
  // ordena vetor auxiliar (insertion sort)
  for (int i = 1; i <= tamanho; i++)
  {
    Cliente *temp = aux[i];
    int j = i - 1;
    while (j >= 0 && aux[j]->cod_cliente > temp->cod_cliente)
    {
      aux[j + 1] = aux[j];
      j--;
    }
    aux[j + 1] = temp;
  }
}

void redistribuirNos(FILE *arqDados, No *noExc, No *noIrmao, No *noPai, int idxPai, int pont)
{
  // vetor de clientes de tamanho no + irmao + 1 (pai)
  Cliente *aux[noExc->m + noIrmao->m + 1];
  // contador para salvar todos os clientes em aux
  int k = 0;
  while (k < noExc->m)
  {
    aux[k] = cliente(noExc->clientes[k]->cod_cliente, noExc->clientes[k]->nome);
    k++;
  }
  // contador auxiliar para contar os M elementos do irmao
  int l = 0;
  while (l < noIrmao->m)
  {
    aux[k] = cliente(noIrmao->clientes[l]->cod_cliente, noIrmao->clientes[l]->nome);
    k++;
    l++;
  }
  // na última posição do vetor se guarda o cliente do pai
  aux[k] = cliente(noPai->clientes[idxPai]->cod_cliente, noPai->clientes[idxPai]->nome);

  insertionSort(aux, k);
  // redistribuir as chaves
  // D chaves no nó
  int cont = 0;
  for (int a = 0; a < D; a++)
  {
    noExc->clientes[a] = cliente(aux[cont]->cod_cliente, aux[cont]->nome);
    noExc->m = a + 1;
    cont++;
  }
  // D + 1 vai para o pai
  noPai->clientes[idxPai] = cliente(aux[cont]->cod_cliente, aux[cont]->nome);
  cont++;
  // restante para o irmão
  int contM = 0;
  for (int a = 0; a < k - D; a++)
  {
    noIrmao->clientes[a] = cliente(aux[cont]->cod_cliente, aux[cont]->nome);
    contM++;
    cont++;
  }
  noIrmao->m = contM;

  // salva novo no pai e novo no irmao
  fseek(arqDados, noExc->pont_pai, SEEK_SET);
  salva_no(noPai, arqDados);
  fseek(arqDados, pont + tamanho_no(), SEEK_SET);
  salva_no(noIrmao, arqDados);
}

void balancearArvore(FILE *arqDados, No *noExc, int pont)
{
  // checa se é necessário balancear
  if (noExc->m >= D || noExc->pont_pai == -1)
  {
    return; // Nó tem no mínimo D elementos ou é raiz, não precisa balancear
  }
  fseek(arqDados, pont + tamanho_no(), SEEK_SET);
  No *noIrmao = le_no(arqDados);

  if (noExc->pont_pai == noIrmao->pont_pai && noExc->m < D && noIrmao != NULL)
  {
    fseek(arqDados, noExc->pont_pai, SEEK_SET);
    No *noPai = le_no(arqDados);
    int idxPai = encontrarIndicePai(noPai, pont);

    if (noExc->m + noIrmao->m >= 2 * D)
    {
      redistribuirNos(arqDados, noExc, noIrmao, noPai, idxPai, pont);
    }
    else
    {
      concatenarNos(arqDados, noExc, noIrmao, noPai, idxPai, pont);
      balancearArvore(arqDados, noPai, noExc->pont_pai);
    }

    libera_no(noPai);
  }

  if (noIrmao)
    libera_no(noIrmao);
}

void concatenarNos(FILE *arq, No *no, No *irmao, No *pai, int idxPai, int pont)
{
  for (int i = 0; i < irmao->m; i++)
  {
    no->clientes[no->m + i] = cliente(irmao->clientes[i]->cod_cliente, irmao->clientes[i]->nome);
    no->p[no->m + i] = irmao->p[i];
  }
  no->m += irmao->m;

  no->clientes[no->m] = cliente(pai->clientes[idxPai]->cod_cliente, pai->clientes[idxPai]->nome);
  no->m++;

  for (int a = idxPai; a < pai->m + 1; a++)
  {
    pai->clientes[a] = pai->clientes[a + 1];
    if (pont != pai->p[a])
    {
      pai->p[a] = pai->p[a + 1];
    }
  }
  pai->m--;

  insertionSort(no->clientes, no->m - 1);

  fseek(arq, no->pont_pai, SEEK_SET);
  salva_no(pai, arq);
}

int encontrarIndicePai(No *pai, int pont)
{
  for (int j = 0; j < pai->m; j++)
  {
    if (pai->p[j] == pont)
    {
      return j;
    }
  }
  return -1;
}

int exclui(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados)
{
  int pont, encontrou = 0;
  int i = busca(cod_cli, nome_arquivo_metadados, nome_arquivo_dados, &pont, &encontrou);

  FILE *arqDados = fopen(nome_arquivo_dados, "rb+");
  if (!arqDados)
  {
    printf("Erro ao abrir arquivo de dados.\n");
    return -1;
  }

  fseek(arqDados, pont, SEEK_SET);
  No *noExc = le_no(arqDados);

  if (encontrou && noExc != NULL)
  {
    if (eh_folha(noExc))
    {
      removeNoFolha(arqDados, noExc, i, pont);
    }
    else
    {
      substituiPorSucessor(arqDados, noExc, i);
    }

    balancearArvore(arqDados, noExc, pont);

    fseek(arqDados, pont, SEEK_SET);
    salva_no(noExc, arqDados);
    libera_no(noExc);
  }

  fclose(arqDados);
  return pont;
}