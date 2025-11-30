#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200
#define ITERATIONS 1000 
#define FILENAME "dataset_id_ascending.csv"

#define INSERTION_THRESHOLD 15 

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char gender;
    int korean;
    int english;
    int math;
    long long gradeSum;
} Student;

typedef struct AVLNode {
    Student data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

long long comparison_count = 0;

typedef enum {
    CRITERIA_ID = 0,
    CRITERIA_NAME,
    CRITERIA_GENDER,
    CRITERIA_GRADES
} Criteria;

int compare_students(Student a, Student b, Criteria criteria, int ascending);
void copy_array(Student* dest, Student* src, int count);
void swap(Student* a, Student* b);
int has_duplicates(Student* arr, int n, Criteria c);

void shell_sort_improved(Student* arr, int n, Criteria c, int asc);
void quick_sort_improved(Student* arr, int low, int high, Criteria c, int asc);
void tree_sort_improved(Student* arr, int n, Criteria c, int asc);

Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) return NULL;

    char line[MAX_LINE_LEN];
    int capacity = 10, count = 0;
    Student* arr = (Student*)malloc(sizeof(Student) * capacity);
    if (!arr) { fclose(fp); return NULL; }

    fgets(line, sizeof(line), fp); // 헤더 건너뛰기
    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = (Student*)realloc(arr, sizeof(Student) * capacity);
            if (!temp) { free(arr); fclose(fp); return NULL; }
            arr = temp;
        }
        Student s;
        memset(&s, 0, sizeof(Student));
        char* token = strtok(line, ",");
        if (token) s.id = atoi(token);
        token = strtok(NULL, ",");
        if (token) {
            strncpy(s.name, token, MAX_NAME_LEN - 1);
            s.name[strcspn(s.name, "\r\n")] = 0;
        }
        token = strtok(NULL, ",");
        if (token) s.gender = token[0];
        token = strtok(NULL, ",");
        if (token) s.korean = atoi(token);
        token = strtok(NULL, ",");
        if (token) s.english = atoi(token);
        token = strtok(NULL, ",");
        if (token) s.math = atoi(token);
        s.gradeSum = (long long)s.korean + s.english + s.math;
        arr[count++] = s;
    }
    fclose(fp);
    *out_count = count;
    return arr;
}


int compare_for_dup(const void* a, const void* b, Criteria c) {
    Student* sa = (Student*)a;
    Student* sb = (Student*)b;
    switch (c) {
    case CRITERIA_ID: return (sa->id > sb->id) - (sa->id < sb->id);
    case CRITERIA_NAME: return strcmp(sa->name, sb->name);
    case CRITERIA_GENDER: return (sa->gender > sb->gender) - (sa->gender < sb->gender);
    case CRITERIA_GRADES:
        if (sa->gradeSum != sb->gradeSum) return (sa->gradeSum > sb->gradeSum) - (sa->gradeSum < sb->gradeSum);
        if (sa->korean != sb->korean) return (sa->korean > sb->korean) - (sa->korean < sb->korean);
        if (sa->english != sb->english) return (sa->english > sb->english) - (sa->english < sb->english);
        return (sa->math > sb->math) - (sa->math < sb->math);
    }
    return 0;
}

int cmp_id(const void* a, const void* b) { return compare_for_dup(a, b, CRITERIA_ID); }
int cmp_name(const void* a, const void* b) { return compare_for_dup(a, b, CRITERIA_NAME); }
int cmp_gender(const void* a, const void* b) { return compare_for_dup(a, b, CRITERIA_GENDER); }
int cmp_grades(const void* a, const void* b) { return compare_for_dup(a, b, CRITERIA_GRADES); }

int has_duplicates(Student* arr, int n, Criteria c) {
    Student* temp = (Student*)malloc(sizeof(Student) * n);
    if (!temp) return 0;
    memcpy(temp, arr, sizeof(Student) * n);
    switch (c) {
    case CRITERIA_ID: qsort(temp, n, sizeof(Student), cmp_id); break;
    case CRITERIA_NAME: qsort(temp, n, sizeof(Student), cmp_name); break;
    case CRITERIA_GENDER: qsort(temp, n, sizeof(Student), cmp_gender); break;
    case CRITERIA_GRADES: qsort(temp, n, sizeof(Student), cmp_grades); break;
    }
    int has_dup = 0;
    for (int i = 0; i < n - 1; i++) {
        if (compare_for_dup(&temp[i], &temp[i + 1], c) == 0) {
            has_dup = 1;
            break;
        }
    }
    free(temp);
    return has_dup;
}

int main() {
    Student* data = NULL;
    Student* buffer = NULL;
    int count = 0;

    data = load_students(FILENAME, &count);
    if (!data || count == 0) {
        printf("Failed to load data or empty file.\n");
        printf("Ensure '%s' is in the correct directory.\n", FILENAME);
        return 1;
    }

    buffer = (Student*)malloc(sizeof(Student) * count);

    Criteria criteria[] = { CRITERIA_ID, CRITERIA_NAME, CRITERIA_GENDER, CRITERIA_GRADES };
    const char* names[] = { "ID", "NAME", "GENDER", "GRADES" };

    printf("================================================================\n");
    printf("            ASSIGNMENT B - OPTIMIZED ALGORITHMS\n");
    printf("   Shell (Ciura) / Quick (Median-3 + Insert) / Tree (AVL)\n");
    printf("                  %d iterations each\n", ITERATIONS);
    printf("================================================================\n\n");

    for (int c = 0; c < 4; c++) {
        int dup = has_duplicates(data, count, criteria[c]);

        for (int asc = 0; asc < 2; asc++) {
            printf("[%s - %s]\n", names[c], asc ? "ASC" : "DESC");
            printf("----------------------------------------------------------------\n");
            printf("%-20s %18s %16s\n", "Algorithm", "Comparisons", "Memory");
            printf("----------------------------------------------------------------\n");

            long long comp = 0;
            for (int k = 0; k < ITERATIONS; k++) {
                copy_array(buffer, data, count);
                comparison_count = 0;
                shell_sort_improved(buffer, count, criteria[c], asc);
                if (k == 0) comp = comparison_count;
            }
            printf("%-20s %18lld %16d\n", "Shell Sort", comp, 0);

            comp = 0;
            for (int k = 0; k < ITERATIONS; k++) {
                copy_array(buffer, data, count);
                comparison_count = 0;
                quick_sort_improved(buffer, 0, count - 1, criteria[c], asc);
                if (k == 0) comp = comparison_count;
            }
            printf("%-20s %18lld %16d\n", "Quick Sort", comp, 0);

            if (!dup) {
                comp = 0;
                long long mem = 0;
                for (int k = 0; k < ITERATIONS; k++) {
                    copy_array(buffer, data, count);
                    comparison_count = 0;
                    tree_sort_improved(buffer, count, criteria[c], asc);
                    if (k == 0) {
                        comp = comparison_count;
                        mem = (long long)(sizeof(AVLNode) * count);
                    }
                }
                printf("%-20s %18lld %16lld\n", "Tree Sort (AVL)", comp, mem);
            }
            else {
                printf("%-20s %18s %16s\n", "Tree Sort (AVL)", "SKIP (Dup)", "-");
            }
            printf("\n");
        }
    }

    free(buffer);
    free(data);

    printf("================================================================\n");
    printf("Performance Tuning Notes:\n");
    printf("  Shell: Ciura Gap Sequence (Best known gaps)\n");
    printf("  Quick: Median-of-Three Pivot + Insertion Sort Cutoff (Thr: %d)\n", INSERTION_THRESHOLD);
    printf("  Tree:  AVL (Balanced BST, O(N log N))\n");
    printf("================================================================\n");
    return 0;
}

void copy_array(Student* dest, Student* src, int count) {
    memcpy(dest, src, sizeof(Student) * count);
}

void swap(Student* a, Student* b) {
    Student temp = *a; *a = *b; *b = temp;
}

int compare_students(Student a, Student b, Criteria criteria, int ascending) {
    comparison_count++;
    int result = 0;
    switch (criteria) {
    case CRITERIA_ID:
        result = (a.id > b.id) - (a.id < b.id);
        break;
    case CRITERIA_NAME:
        result = strcmp(a.name, b.name);
        break;
    case CRITERIA_GENDER:
        result = (a.gender > b.gender) - (a.gender < b.gender);
        break;
    case CRITERIA_GRADES:
        if (a.gradeSum != b.gradeSum) result = (a.gradeSum > b.gradeSum) ? 1 : -1;
        else if (a.korean != b.korean) result = (a.korean > b.korean) ? 1 : -1;
        else if (a.english != b.english) result = (a.english > b.english) ? 1 : -1;
        else if (a.math != b.math) result = (a.math > b.math) ? 1 : -1;
        break;
    }
    return ascending ? result : -result;
}

void shell_sort_improved(Student* arr, int n, Criteria c, int asc) {
    int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
    int ng = 8;

    for (int g = 0; g < ng; g++) {
        int gap = gaps[g];
        if (gap >= n) continue; // 배열 크기보다 큰 간격은 패스

        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            int j;
            for (j = i; j >= gap && compare_students(arr[j - gap], temp, c, asc) > 0; j -= gap) {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }
}

void insertion_sort_for_quick(Student* arr, int low, int high, Criteria c, int asc) {
    for (int i = low + 1; i <= high; i++) {
        Student key = arr[i];
        int j = i - 1;
        while (j >= low && compare_students(arr[j], key, c, asc) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void quick_sort_improved(Student* arr, int low, int high, Criteria c, int asc) {
    if (low >= high) return;

    if (high - low <= INSERTION_THRESHOLD) {
        insertion_sort_for_quick(arr, low, high, c, asc);
        return;
    }

    int mid = low + (high - low) / 2;

    if (compare_students(arr[mid], arr[low], c, asc) < 0) swap(&arr[low], &arr[mid]);
    if (compare_students(arr[high], arr[low], c, asc) < 0) swap(&arr[low], &arr[high]);
    if (compare_students(arr[high], arr[mid], c, asc) < 0) swap(&arr[mid], &arr[high]);

    Student pivot = arr[mid];

    int i = low;
    int j = high;

    while (i <= j) {
        while (compare_students(arr[i], pivot, c, asc) < 0) i++;
        while (compare_students(arr[j], pivot, c, asc) > 0) j--;
        if (i <= j) {
            swap(&arr[i], &arr[j]);
            i++;
            j--;
        }
    }

    if (low < j) quick_sort_improved(arr, low, j, c, asc);
    if (i < high) quick_sort_improved(arr, i, high, c, asc);
}

int max_int(int a, int b) { return (a > b) ? a : b; }
int get_height(AVLNode* n) { return n ? n->height : 0; }
int get_balance(AVLNode* n) { return n ? get_height(n->left) - get_height(n->right) : 0; }

AVLNode* create_node(Student data) {
    AVLNode* n = (AVLNode*)malloc(sizeof(AVLNode));
    n->data = data;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

AVLNode* rotate_right(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    return x;
}

AVLNode* rotate_left(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max_int(get_height(x->left), get_height(x->right)) + 1;
    y->height = max_int(get_height(y->left), get_height(y->right)) + 1;
    return y;
}

AVLNode* insert_avl(AVLNode* node, Student data, Criteria c, int asc) {
    if (!node) return create_node(data);

    int cmp = compare_students(data, node->data, c, asc);

    if (cmp < 0) node->left = insert_avl(node->left, data, c, asc);
    else if (cmp > 0) node->right = insert_avl(node->right, data, c, asc);
    else return node; 

    node->height = 1 + max_int(get_height(node->left), get_height(node->right));
    int bal = get_balance(node);

    if (bal > 1) {
        if (compare_students(data, node->left->data, c, asc) < 0)
            return rotate_right(node);
        else {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }
    }
    if (bal < -1) {
        if (compare_students(data, node->right->data, c, asc) > 0)
            return rotate_left(node);
        else {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }
    }
    return node;
}

void inorder(AVLNode* node, Student* arr, int* idx) {
    if (node) {
        inorder(node->left, arr, idx);
        arr[(*idx)++] = node->data;
        inorder(node->right, arr, idx);
    }
}

void free_tree(AVLNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        free(node);
    }
}

void tree_sort_improved(Student* arr, int n, Criteria c, int asc) {
    AVLNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = insert_avl(root, arr[i], c, asc);
    }
    int idx = 0;
    inorder(root, arr, &idx);
    free_tree(root);
}