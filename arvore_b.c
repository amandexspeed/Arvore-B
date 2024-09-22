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
/* No* verica_nos_pai_cheio(No *no) {
  if(no->pont_pai == NULL) return NULL; // chegou na raiz
  if (esta_cheio) verica_nos_pai_cheio(no->pont_pai); //verifica se o pai está cheio
  else return no; // se não estiver cheio retorna esse no, que é o que vai ser feito o particionamento
}
 */

/* Particiona */
/* 
  -> Função responsável por particionar o nó
  -> o nó P recebe fica com uma posição a mais
  -> depois é ordenado
  -> todas as chaves depois do meio são passadas para o nó Q
  -> a chave do meio é removida do nó P e retornada, pois será promovida ao nó pai
*/
Cliente* particiona(No *p, No *q, int meio_p, Cliente *novo_cliente) {
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
  p->p[meio_p] = -1;
  
  
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
int insere_em_raiz_cheia(No *p, No *q, Metadados *m_dados, int posicao_livre, FILE *arq_dados, char *nome_arquivo_metadados, Cliente *chave_promovida) {
  No *nova_raiz = no(1,-1);
  nova_raiz->clientes[0] = chave_promovida;

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
  
  if (!esta_cheio(no_atual)) {
    no_atual->clientes[no_atual->m] = novo_cliente;
    no_atual->m++;
    ordenar_no(no_atual);
    fseek(arqDados, pontChave, SEEK_SET);
    salva_no(no_atual, arqDados);
    fclose(arqDados);
    return pontChave;
  }

  // Está cheio, deve-se fazer o particionamento
  No *novo_no = no(0, 0);
  int pont_chave_no_atual = pontChave;  // Guarda chave do nó atual
  int meio = ((no_atual->m+1) / 2);     // Posição do meio do nó
  Metadados *m_dados;
  int posicao_livre;
  Cliente *chave_promovida = particiona(no_atual, novo_no, meio, novo_cliente);

  if(no_atual->pont_pai != -1) {
    // Atualizar o ponteiro do novo nó para um endereço correto
    m_dados = le_arq_metadados(nome_arquivo_metadados);
    posicao_livre = m_dados->pont_prox_no_livre; 
    m_dados->pont_prox_no_livre+=tamanho_no();
    salva_arq_metadados(nome_arquivo_metadados,m_dados);
    
    // Busca o pai da chave promovida
    fseek(arqDados, no_atual->pont_pai, SEEK_SET);
    No *no_pai = le_no(arqDados);
    
    // Inserir a chave promovida no nó pai
    no_pai->clientes[no_pai->m] = chave_promovida;
    no_pai->p[no_pai->m+1] = posicao_livre;  // Atualizar ponteiro para o novo nó
    no_pai->m++;
    ordenar_no(no_pai);

    // Salvar nó pai atualizado
    fseek(arqDados, no_atual->pont_pai, SEEK_SET);
    salva_no(no_pai, arqDados);

    // Salvar nó atual atualizado
    fseek(arqDados, pont_chave_no_atual, SEEK_SET);
    salva_no(no_atual, arqDados);

    // Salvar novo nó
    fseek(arqDados, posicao_livre, SEEK_SET);
    salva_no(novo_no, arqDados);
    
    fclose(arqDados);
    
    return pontChave;
  }

  // caso o no seja raíz
  posicao_livre = insere_em_raiz_cheia(no_atual, novo_no, m_dados, posicao_livre, arqDados, nome_arquivo_metadados, chave_promovida);
  
  return pontChave;
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

  fclose(arqDados); // Fecha o arquivo após salvar

  return pont;
}