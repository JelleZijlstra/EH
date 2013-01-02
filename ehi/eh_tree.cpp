/*
 * eh_tree.c
 * Jelle Zijlstra, December 2011
 *
 * Functionality to create and show the EH abstract syntax tree.
 */
#include "eh.hpp"
#include "eh_tree.hpp"
#include <stdio.h>

Node::t *eh_addnode(int opcode) {
	return new Node::t(opcode, 0);
}
Node::t *eh_addnode(int opcode, ehval_p first) {
	Node::t *op = new Node::t(opcode, 1);
	op->paras[0] = first;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second) {
	Node::t *op = new Node::t(opcode, 2);
	op->paras[0] = first;
	op->paras[1] = second;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third) {
	Node::t *op = new Node::t(opcode, 3);
	op->paras[0] = first;
	op->paras[1] = second;
	op->paras[2] = third;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth) {
	Node::t *op = new Node::t(opcode, 4);
	op->paras[0] = first;
	op->paras[1] = second;
	op->paras[2] = third;
	op->paras[3] = fourth;
	return op;
}
