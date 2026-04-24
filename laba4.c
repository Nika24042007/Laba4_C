#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Key{
    char key[6];
    double num;
} Key;

typedef struct Node{
    int leaf;
    int n;
    struct Node *parent;
    Key *keys[3];
    struct Node *child[4];
    int c;
} Node;

Node *Create_node(){
    Node *node = malloc(sizeof(Node));
    node->leaf = 1;
    node->n = 0;
    node->c = 0;
    node->parent = NULL;
    for (int i = 0; i< 4; i++){
        node->child[i] = NULL;
    }
    for (int i = 0; i <3; i++){
        node->keys[i] = NULL;
    }
    return node;
}

Key *Create_key(double n, char key[6]){
    Key *k = malloc(sizeof(Key));
    if (k == NULL){
        printf("Memory error\n");
        free(k);
        return NULL;
    }
    k->num = n;
    strcpy(k->key, key);
    return k;

}
int comp(const void *a, const void *b) {
    Key *keyA = *(Key **)a;
    Key *keyB = *(Key **)b;
    return strcmp(keyA->key, keyB->key);
}

void Sort(Key **keys, int n){
    qsort(keys, n, sizeof(keys[0]), comp);
}

void Add_in_nofull_node(Node *node, Key *k){
    node->keys[node->n] = k;
    node->n++;
    if (node->n > 1){
        Sort(node->keys, node->n);
    }
}

void Print_tree(Node *root, int level, FILE *w) {
    if (root == NULL) {
        if (level ==0){
            fprintf(w, "Empty tree\n");
        }
        return;
    }
    Print_tree(root->child[0], level + 1, w);

    for (int j = 0; j < root->n; j++) {
        int a = root->n;
        if (level > 0) {
            for (int i = 0; i < level; i++) {
                fprintf(w, "|");
                if (i != (level - 1))
                    fprintf(w, "   ");
            }
        }
        if (root->keys[j] != NULL){
            fprintf(w, "%s %.2f\n", root->keys[j]->key, root->keys[j]->num);
        }
        Print_tree(root->child[j + 1], level + 1, w);
    }
}


void Delete_from_leaf(Node *leaf, int pos) {
    for (int i = pos; i < leaf->n-1; i++) {
        leaf->keys[i] = leaf->keys[i+1];
    }
    leaf->keys[leaf->n - 1] = NULL;
    leaf->n--;
}

void Borrow_from_left(Node *parent, int index) {
    Node *node = parent->child[index];
    Node *left = parent->child[index - 1];
    for (int i = node->n; i > 0; i--) {
        node->keys[i] = node->keys[i-1];
    }
    node->keys[0] = parent->keys[index - 1];
    node->n++;
    parent->keys[index - 1] = left->keys[left->n - 1];
    left->keys[left->n - 1] = NULL;
    left->n--;
    if (node->leaf != 1) {
        for (int i = node->n; i > 0; i--) {
            node->child[i] = node->child[i-1];
        }
        node->child[0] = left->child[left->n + 1];
        left->child[left->n + 1] = NULL;
        node->c++;
        left->c--;
        node->child[0]->parent = node;
    }
}

void Borrow_from_right(Node *parent, int index) {
    Node *node = parent->child[index];
    Node *right = parent->child[index + 1];
    node->keys[node->n] = parent->keys[index];
    node->n++;
    parent->keys[index] = right->keys[0];
    for (int i = 0; i < right->n - 1; i++) {
        right->keys[i] = right->keys[i+1];
    }
    right->keys[right->n-1] = NULL;
    right->n--;

    if (node->leaf != 1) {
        node->child[node->n] = right->child[0];
        for (int i = 0; i < right->n + 1; i++) {
            right->child[i] = right->child[i+1];
        }
        right->child[right->n+1] = NULL;
        node->c++;
        right->c--;
        node->child[node->n]->parent = node;
    }
}

void Merge(Node *parent, int index) {
    Node *left = parent->child[index];
    Node *right = parent->child[index + 1];
    left->keys[left->n] = parent->keys[index];
    left->n++;
    for (int i = 0; i < right->n; i++) {
        left->keys[left->n + i] = right->keys[i];
    }
    left->n = left->n + right->n;

    if (left->leaf != 1) {
        for (int i = 0; i <= right->n; i++) {
            left->child[left->n - right->n + i] = right->child[i];
            if (right->child[i]) right->child[i]->parent = left;
        }
        left->c = left->n + 1;
    }
    for (int i = index; i < parent->n - 1; i++) {
        parent->keys[i] = parent->keys[i+1];
    }
    parent->keys[parent->n - 1] = NULL;
    parent->n--;
    for (int i = index + 1; i < parent->c - 1; i++) {
        parent->child[i] = parent->child[i+1];
    }
    parent->child[parent->c - 1] = NULL;
    parent->c--;
    free(right);
    
    if (parent->parent == NULL && parent->n == 0) {
        left->parent = NULL;
        parent->leaf = left->leaf;
        parent->n = left->n;
        parent->c = left->c;
        for (int i = 0; i < left->n; i++) parent->keys[i] = left->keys[i];
        for (int i = 0; i <= left->n; i++) {
            parent->child[i] = left->child[i];
            if (parent->child[i] != NULL){
                parent->child[i]->parent = parent;
            }
        }

        free(left);
    }
}

void Balance_internal(Node *node);

void Balance_leaf(Node *leaf) {
    Node *parent = leaf->parent;
    if (parent == NULL) return;

    int pos;
    for (int i = 0; i < parent->c; i++) {
        if (parent->child[i] == leaf) {
            pos = i;
            break;
        }
    }

    if (pos > 0 && parent->child[pos-1]->n > 1) {
        Borrow_from_left(parent, pos);
    }
    else if (pos < parent->c - 1 && parent->child[pos+1]->n > 1) {
        Borrow_from_right(parent, pos);
    }
    else {
        if (pos > 0) {
            Merge(parent, pos-1);
        } else {
            Merge(parent, pos);
        }
        if (parent->parent != NULL && parent->n < 1) {
            Balance_internal(parent);
        }
    }
}


void Balance_internal(Node *node) {
    Node *parent = node->parent;
    if (parent == NULL) {
        if (node->c == 1) {
            Node *child = node->child[0];
            child->parent = NULL;
            node->leaf = child->leaf;
            node->n = child->n;
            node->c = child->c;
            for (int i = 0; i < child->n; i++) node->keys[i] = child->keys[i];
            for (int i = 0; i <= child->n; i++) {
                node->child[i] = child->child[i];
                node->child[i]->parent = node;
            }
            free(child);
        }
        return;
    }
    int pos;
    for (pos = 0; pos < parent->c; pos++) {
        if (parent->child[pos] == node) break;
    }
    if (pos > 0 && parent->child[pos-1]->n > 1) {
        Borrow_from_left(parent, pos);
    }else if (pos < parent->c - 1 && parent->child[pos+1]->n > 1) {
        Borrow_from_right(parent, pos);
    }else {
        if (pos > 0) {
            Merge(parent, pos-1);
        } else {
            Merge(parent, pos);
        }
        if (parent->parent != NULL && parent->n < 1) {
            Balance_internal(parent);
        }
    }
}

void Delete_from_internal(Node *node, int pos) {
    Key *k = node->keys[pos];
    Node *left_child = node->child[pos];
    Node *right_child = node->child[pos+1];

    if (left_child->n >= 2) {
        Node *cur = left_child;
        while (cur->leaf != 1) {
            cur = cur->child[cur->n];
        }
        Key *pred = cur->keys[cur->n - 1];
        node->keys[pos] = pred;
        Delete_from_leaf(cur, cur->n - 1);
        if (cur->n < 1 && cur->parent != NULL){
            Balance_leaf(cur);
        }
    }
    else if (right_child->n >= 2) {
        Node *cur = right_child;
        while (cur->leaf != 1){
            cur = cur->child[0];
        }
        Key *pred = cur->keys[0];
        node->keys[pos] = pred;
        Delete_from_leaf(cur, 0);
        if (cur->n < 1 && cur->parent != NULL){
            Balance_leaf(cur);
        }
    }
    else {
        Merge(node, pos);
        if (node->parent != NULL && node->n < 1) {
            Balance_internal(node);
        }
    }
}

void Delete(Node **root, Key *k, FILE *w) {
    Node *node = NULL;
    Node *prt = *root;
    int p;
    if (*root == NULL){
        return;
    }
    for (int i = 0; i < prt->n; i++){
        int cm = strcmp(k->key, prt->keys[i]->key);
        if (cm == 0){
            node = prt;
            p = i;
            break;
        }
        else if(cm < 0 ){
            prt = prt->child[i];
            i = -1;
        }
        else if(i == prt->n-1){
            if(cm > 0){
                prt = prt->child[i+1];
                i = -1;
            }else{
                fprintf(w, "Error: no such key1\n");
                return;
            }
        }
        if( prt == NULL){
            fprintf(w, "Error: no such key2\n");
            return;
        }
    }
    if (node->leaf) {
        Delete_from_leaf(node, p);
        if (node->n < 1 && node->parent != NULL) {
            Balance_leaf(node);
        } else if (node->parent == NULL && node->n == 0) {
            free(node);
            *root = NULL;
        }
    }else {
        Delete_from_internal(node, p);
    }

}

void Cut_node(Node *node) {
    if (node->n < 3) return;
    Key *mid_key = node->keys[1];

    Node *left = Create_node();
    Node *right = Create_node();
    left->keys[0] = node->keys[0];
    left->n = 1;
    right->keys[0] = node->keys[2];
    right->n = 1;

    if (node->leaf != 1) {
        left->child[0] = node->child[0];
        left->child[1] = node->child[1];
        left->c = 2;
        left->leaf = 0;
        if (node->child[0]) node->child[0]->parent = left;
        if (node->child[1]) node->child[1]->parent = left;

        right->child[0] = node->child[2];
        right->child[1] = node->child[3];
        right->c = 2;
        right->leaf = 0;
        if (node->child[2]) node->child[2]->parent = right;
        if (node->child[3]) node->child[3]->parent = right;
    } else {
        left->leaf = 1;
        left->c = 0;
        right->leaf = 1;
        right->c = 0;
    }

    if (node->parent == NULL) {
        Node *new_root = Create_node();
        new_root->keys[0] = mid_key;
        new_root->n = 1;
        new_root->leaf = 0;
        new_root->c = 2;
        new_root->child[0] = left;
        new_root->child[1] = right;
        left->parent = node;
        right->parent = node;
        node->n = new_root->n;
        node->leaf = new_root->leaf;
        node->c = new_root->c;
        node->keys[0] = new_root->keys[0];
        node->child[0] = new_root->child[0];
        node->child[1] = new_root->child[1];
        node->parent = NULL;
        for (int i = 2; i < 4; i++) node->child[i] = NULL;
        for (int i = 1; i < 3; i++) node->keys[i] = NULL;
        free(new_root);
    } else {
        Node *parent = node->parent;
        int pos;
        for (pos = 0; pos <= parent->n; pos++) {
            if (parent->child[pos] == node) break;
        }

        for (int i = parent->n; i > pos; i--) {
            parent->keys[i] = parent->keys[i-1];
        }
        parent->keys[pos] = mid_key;
        parent->n++;

        for (int i = parent->n; i > pos+1; i--) {
            parent->child[i] = parent->child[i-1];
        }
        parent->child[pos] = left;
        parent->child[pos+1] = right;
        left->parent = parent;
        right->parent = parent;
        parent->c = parent->n + 1;
        free(node);

    }
}

Node *Add_in_node_with_search(Node *node, Key *k) {
    if (node == NULL) {
        Node *new_node = Create_node();
        new_node->keys[0] = k;
        new_node->n = 1;
        return new_node;
    }

    Node *cur = node;
    while (cur->leaf != 1) {
        int i;
        for (i = 0; i < cur->n; i++) {
            int cmp = strcmp(k->key, cur->keys[i]->key);
            if (cmp < 0) {
                cur = cur->child[i];
                break;
            }
            if (i == cur->n - 1) {
                cur = cur->child[i + 1];
                break;
            }
        }
    }
    Add_in_nofull_node(cur, k);

    while (cur != NULL && cur->n == 3) {
        Node *parent = cur->parent; 
        Cut_node(cur);
        if (parent == NULL) {
            node = cur;
            break;
        }
        cur = parent;
    }
    
    return node;
}

void Search(Node *root, char *key, FILE* w){
    if (root == NULL){
        fprintf(w, "Error: tree is empty\n");
        return;
    }
    Node *ptr = root;
    for (int i = 0; i < ptr->n; i++){
        if (strcmp(key, ptr->keys[i]->key) == 0){
            fprintf(w, "Search: %s %f\n", ptr->keys[i]->key, ptr->keys[i]->num);
            return;
        }
        else if (strcmp(key, ptr->keys[i]->key) < 0){
            ptr = ptr->child[i];
            i = -1;
        }
        else if(i == ptr->n-1){
            if(strcmp(key, ptr->keys[i]->key) > 0){
                ptr = ptr->child[i+1];
                i = -1;
            }else{
                fprintf(w, "Error: no such key in tree\n");
                return;
            }
        }
        if (ptr == NULL){
            fprintf(w, "Error: no such key in tree\n");
            return;
        }
    }
}

void Free_tree(Node* root){
    if (root == NULL) return;
    if(root->leaf != 1){
        for(int i=0; i <root->c; i++){
            Free_tree(root->child[i]);
        }
    }
    for (int i =0; i < root->n; i++){
        free(root->keys[i]);
    }
    free(root);
}

void main(){
    FILE* f;
    FILE* w;
    f = fopen("Exemple.txt", "r");
    int command;
    char key[6];
    double num;
    if (f == NULL){
        printf("Error in open file\n");
        fclose(f);
        return;
    }
    fscanf(f, "%d ", &command);
    Node *root = NULL;
    w = fopen("Res.txt", "w");
    if (w == NULL){
        printf("Error in open file\n");
        fclose(f);
        return;
    }
    while (command != EOF){
        if (command == 1){
            fscanf(f, "%6s %lf\n", key, &num);
            Key *k = Create_key(num, key);
            root = Add_in_node_with_search(root, k);
        }else if(command == 2){
            fscanf(f, "%6s\n", key);
            num = 0;
            Key *k = Create_key(num, key);
            Delete(&root, k, w);
        }else if(command == 3){
            fscanf(f,"%6s\n", key);
            Search(root, key, w);
        }else if(command == 4){
            Print_tree(root, 0, w);
            fprintf(w, "\n--------------------------------------------------\n");
        }else if(command == 0){
            break;
        }else{
            printf("Error: no such command\n");
        }
        fscanf(f, "%d ", &command);
    }
    fclose(f);
    fprintf(w, "\nEnd");
    fclose(w);
    Free_tree(root);
}