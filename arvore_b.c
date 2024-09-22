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

int insere(int cod_cli, char *nome_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados)
{
  // TODO: Inserir aqui o codigo do algoritmo de insercao
  return INT_MAX;
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