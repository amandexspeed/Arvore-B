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

// int posicao(int chave, No *no)
// {
//   int inicio = 0;
//   int fim = no->m;
//   int pos = (fim + inicio) / 2;
//   while (pos != no->m && chave != no->p[pos] && inicio < fim)
//   {
//     if (chave > no->p[pos])
//     {
//       inicio = pos + 1;
//     }
//     else
//     {
//       fim = pos;
//     }
//     pos = (fim + inicio) / 2;
//   }
//   return pos;
// }

// int busca(No *no, int chave)
// {
//   int pos = posicao(chave, no);
//   if (no->p[pos] == NULL || (pos < no->m && chave == no->p[pos]))
//   {
//     return no;
//   }
//   else
//   {
//     return busca(no->p[pos], chave);
//   }
// }

// int busca(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados, int *pont, int *encontrou)
// {
//   FILE *metaArq = fopen(nome_arquivo_metadados, "rb");
//   FILE *dataArq = fopen(nome_arquivo_dados, "rb");

//   if (!metaArq || !dataArq)
//   {
//     printf("Erro ao abrir os arquivos.\n");
//     *pont = INT_MAX;
//     *encontrou = false;
//     return -1;
//   }

//   Metadados meta;
//   fread(&meta, sizeof(Metadados), 1, metaArq);
//   int pos_no_atual = meta.pont_raiz;

//   while (pos_no_atual != -1)
//   {
//     No *no = le_no(dataArq);
//     int i = busca(no, cod_cli);
//     if (i < no->m && no->clientes[i]->cod_cliente == cod_cli)
//     {
//       *pont = pos_no_atual;
//       *encontrou = true;
//       free(no);
//       fclose(metaArq);
//       fclose(dataArq);
//       return 1;
//     }
//     else if (no->p[i] == -1)
//     {
//       *pont = pos_no_atual;
//       *encontrou = false;
//       free(no);
//       fclose(metaArq);
//       fclose(dataArq);
//       return 0;
//     }
//     else
//     {
//       pos_no_atual = no->p[i];
//     }
//     free(no);
//   }

//   *pont = INT_MAX;
//   *encontrou = false;
//   return 0;
// }

int insere(int cod_cli, char *nome_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados)
{
  // TODO: Inserir aqui o codigo do algoritmo de insercao
  return INT_MAX;
}

int exclui(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados)
{
  int pont;
  int encontrou = 0;

  // Busca a chave no arquivo de dados
  int i = busca(cod_cli, nome_arquivo_metadados, nome_arquivo_dados, &pont, &encontrou);

  FILE *arqDados = fopen(nome_arquivo_dados, "rb+");
  fseek(arqDados, pont, SEEK_SET);
  No *no = le_no(arqDados);

  if (encontrou && no != NULL)
  {
    if (eh_folha(no))
    {
      // Caso 1: A chave está em um nó folha, remove diretamente
      while (i < no->m - 1)
      {
        no->clientes[i] = no->clientes[i + 1];
        i++;
      }
      no->clientes[no->m - 1] = NULL; // Remove a última chave
      no->m--;                        // Decrementa o número de chaves no nó
    }
    else
    {
      // Caso 2: A chave não está em um nó folha
      // Substitui pela chave sucessora
      int sucessor_pont = no->p[i + 1]; // Pega o ponteiro para o nó à direita da chave
      fseek(arqDados, sucessor_pont, SEEK_SET);
      No *no_sucessor = le_no(arqDados);

      while (!eh_folha(no_sucessor))
      {
        // Desce até encontrar um nó folha
        sucessor_pont = no_sucessor->p[0]; // Vai para o primeiro ponteiro
        fseek(arqDados, sucessor_pont, SEEK_SET);
        no_sucessor = le_no(arqDados);
      }

      // Substitui a chave removida pela chave sucessora
      no->clientes[i] = no_sucessor->clientes[0];

      // Remove a chave sucessora do nó folha
      for (int j = 0; j < no_sucessor->m - 1; j++)
      {
        no_sucessor->clientes[j] = no_sucessor->clientes[j + 1];
      }
      no_sucessor->clientes[no_sucessor->m - 1] = NULL;
      no_sucessor->m--;

      // Reposiciona o ponteiro e salva o nó sucessor atualizado
      fseek(arqDados, sucessor_pont, SEEK_SET);
      salva_no(no_sucessor, arqDados);

      // Libera a memória do nó sucessor
      libera_no(no_sucessor);
    }

    // Reposiciona o ponteiro antes de salvar o nó modificado
    fseek(arqDados, pont, SEEK_SET);
    salva_no(no, arqDados);

    libera_no(no);
  }

  Metadados *mDados = le_arq_metadados(nome_arquivo_metadados);
  if (mDados->pont_prox_no_livre < pont + tamanho_no())
  {
    fseek(arqDados, pont + tamanho_no(), SEEK_SET);
    No *noIrmao = le_no(arqDados);
    if (no->m + noIrmao->m >= 2 * D)
    {
      // redistribuir
      fseek(arqDados, no->pont_pai, SEEK_SET);
      No *noPai = le_no(arqDados);
      Cliente *aux[no->m + noIrmao->m + 1];
      int k = 0;
      while (k < no->m)
      {
        aux[k] = no->clientes[k];
        k++;
      }
      int l = 0;
      while (k < noIrmao->m)
      {
        aux[k] = noIrmao->clientes[l];
        k++;
        l++;
      }
      int j = 0;
      while (j < noPai->m)
      {
        if (noPai->p[j] == no[i].pont_pai)
          aux[k] = noPai->clientes[j];
        j++;
      }
      j = no->m + noIrmao->m + 1;
      while (j > 0 && aux[j]->cod_cliente < aux[j - 1]->cod_cliente)
      {
        Cliente *temp = aux[j];
        aux[j] = aux[j - 1];
        aux[j - 1] = temp;
        j--;
      }
      // TODO: guardar D chaves no pont, D+1 no pai, e o restante no irmão e testar se funciona. :'(
    }
  }

  fclose(arqDados); // Fecha o arquivo após salvar

  return pont;
}
