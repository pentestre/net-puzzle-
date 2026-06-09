#include "queue.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* *********************************************************** */

// Création d'une nouvelle queue
queue *queue_new() {
  queue *q = malloc(sizeof(queue));
  if (!q) {
    fprintf(stderr, "Memory allocation failed for queue\n");
    exit(EXIT_FAILURE);
  }
  q->length = 0;
  q->tail = q->head = NULL;
  return q;
}

/* *********************************************************** */

// Ajouter un élément au début de la queue
void queue_push_head(queue *q, void *data) {
  assert(q);
  element_t *e = malloc(sizeof(element_t));
  if (!e) {
    fprintf(stderr, "Memory allocation failed for queue element\n");
    exit(EXIT_FAILURE);
  }
  e->data = data;
  e->prev = NULL;
  e->next = q->head;
  if (q->head) q->head->prev = e;
  q->head = e;
  if (!q->tail) q->tail = e;
  q->length++;
}

/* *********************************************************** */

// Ajouter un élément à la fin de la queue
void queue_push_tail(queue *q, void *data) {
  assert(q);
  element_t *e = malloc(sizeof(element_t));
  if (!e) {
    fprintf(stderr, "Memory allocation failed for queue element\n");
    exit(EXIT_FAILURE);
  }
  e->data = data;
  e->prev = q->tail;
  e->next = NULL;
  if (q->tail) q->tail->next = e;
  q->tail = e;
  if (!q->head) q->head = e;
  q->length++;
}

/* *********************************************************** */

// Supprimer un élément au début de la queue
void *queue_pop_head(queue *q) {
  assert(q);
  if (q->length == 0) return NULL;  // Pas d'élément à supprimer

  element_t *head = q->head;
  void *data = head->data;
  q->head = head->next;

  if (q->head) {
    q->head->prev = NULL;
  } else {
    q->tail = NULL;  // Si la queue devient vide
  }

  free(head);
  q->length--;
  return data;
}

/* *********************************************************** */

// Supprimer un élément à la fin de la queue
void *queue_pop_tail(queue *q) {
  assert(q);
  if (q->length == 0) return NULL;  // Pas d'élément à supprimer

  element_t *tail = q->tail;
  void *data = tail->data;
  q->tail = tail->prev;

  if (q->tail) {
    q->tail->next = NULL;
  } else {
    q->head = NULL;  // Si la queue devient vide
  }

  free(tail);
  q->length--;
  return data;
}

/* *********************************************************** */

// Récupérer la longueur de la queue
int queue_length(const queue *q) {
  assert(q);
  return q->length;
}

/* *********************************************************** */

// Vérifier si la queue est vide
bool queue_is_empty(const queue *q) { return q->length == 0; }

/* *********************************************************** */

// Récupérer l'élément au début de la queue sans le retirer
void *queue_peek_head(queue *q) {
  assert(q);
  assert(q->head);
  return q->head->data;
}

/* *********************************************************** */

// Récupérer l'élément à la fin de la queue sans le retirer
void *queue_peek_tail(queue *q) {
  assert(q);
  assert(q->tail);
  return q->tail->data;
}

/* *********************************************************** */

// Vider la queue en ne libérant que les éléments (mais pas les données)
void queue_clear(queue *q) {
  assert(q);
  element_t *e = q->head;
  while (e) {
    element_t *tmp = e;
    e = e->next;
    free(tmp);
  }
  q->head = q->tail = NULL;
  q->length = 0;
}

/* *********************************************************** */

// Vider la queue en libérant aussi les données via la fonction de destruction
void queue_clear_full(queue *q, void (*destroy)(void *)) {
  assert(q);
  element_t *e = q->head;
  while (e) {
    element_t *tmp = e;
    if (destroy) destroy(e->data);  // Appeler destroy si fourni
    e = e->next;
    free(tmp);
  }
  q->head = q->tail = NULL;
  q->length = 0;
}

/* *********************************************************** */

// Libérer la queue (les éléments et la structure)
void queue_free(queue *q) {
  if (!q) return;
  queue_clear(q);  // Vider la queue avant de libérer la structure
  free(q);
}

/* *********************************************************** */

// Libérer la queue (les éléments et la structure) avec destruction des données
void queue_free_full(queue *q, void (*destroy)(void *)) {
  if (!q) return;
  queue_clear_full(q, destroy);  // Vider la queue avant de libérer la structure
  free(q);
}

/* *********************************************************** */
