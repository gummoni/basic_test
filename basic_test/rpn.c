#include<stdio.h>
#include "rpn.h"


void rpn_decode(rpn_info* self, int value) {
  switch (self->state) {
    case 0:
      self->left = value;
      self->state = 1;
      break;

    case 1:
      self->old_op = (char)value;
      self->state = 2;
      break;

    case 2:
      if ('*' == self->old_op) {
        self->left = self->left * value;
        self->state = 1;
      } else if ('/' == self->old_op) {
        self->left = self->left / value;
        self->state = 1;
      } else {
        self->right = value;
        self->state = 3;
      }
      break;

    case 3:
      self->cur_op = (char)value;
      self->state = 4;
      break;

    case 4:
      if ('*' == self->cur_op) {
        self->right = self->right * value;
      } else if ('/' == self->old_op) {
        self->right = self->right / value;
      } else if ('+' == self->old_op) {
        self->left = self->left + self->right;
        self->right = value;
        self->old_op = self->cur_op;
      } else if ('-' == self->old_op) {
        self->left = self->left - self->right;
        self->right = value;
        self->old_op = self->cur_op;
      }
      self->state = 3;
      break;
  }
}

int rpn_result(rpn_info* self) {
  if (3 == self->state) {
    self->state = 0;
    self->left = ('+' == self->old_op) ? (self->left + self->right) : (self->left - self->right);
  }
  return self->left;
}
