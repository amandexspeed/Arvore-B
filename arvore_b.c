#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "arvore_b.h"
#include "metadados.h"
#include "no.h"
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>

int posicao(int chave, No *no) {
  int inicio = 0;
  int fim = no->m;
  int pos = (fim + inicio) / 2;
  while (pos != no->m && chave != no->p[pos] && inicio < fim) {
    if (chave > no->p[pos]) {
      inicio = pos + 1;
    } else {
      fim = pos;
    }
    pos = (fim + inicio) / 2;
  }
  return pos;
}

int busca(No *no, int chave) {
  int pos = posicao(chave, no);
  if (no->p[pos] == NULL || (pos < no->m && chave == no->p[pos])) {
    return no;
  } else {
    return busca(no->p[pos], chave);
  }
}

int busca(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados, int *pont, int *encontrou) {
  FILE *metaArq = fopen(nome_arquivo_metadados, "rb");
  FILE *dataArq = fopen(nome_arquivo_dados, "rb");

  if (!metaArq || !dataArq) {
    printf("Erro ao abrir os arquivos.\n");
    *pont = INT_MAX;
    *encontrou = false;
    return -1;
  }

  Metadados meta;
  fread(&meta, sizeof(Metadados), 1, metaArq);
  int pos_no_atual = meta.pont_raiz;

  while (pos_no_atual != -1) {
    No *no = le_no(dataArq);
    int i = busca(no, cod_cli);
    if (i < no->m && no->clientes[i]->cod_cliente == cod_cli) {
      *pont = pos_no_atual;
      *encontrou = true;
      free(no);
      fclose(metaArq);
      fclose(dataArq);
      return 1;
    } else if (no->p[i] == -1) {
      *pont = pos_no_atual;
      *encontrou = false;
      free(no);
      fclose(metaArq);
      fclose(dataArq);
      return 0;
    } else {
      pos_no_atual = no->p[i];
    }
    free(no);
  }

  *pont = INT_MAX;
  *encontrou = false;
  return 0;
}

int insere(int cod_cli, char *nome_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados) {
  // TODO: Inserir aqui o codigo do algoritmo de insercao
  return INT_MAX;
}

int exclui(int cod_cli, char *nome_arquivo_metadados, char *nome_arquivo_dados) {
  // TODO: Inserir aqui o codigo do algoritmo de remocao
  return INT_MAX;
}
