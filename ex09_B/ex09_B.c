#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200
#define ITERATIONS 1000 // 요청하신 대로 1000회 반복으로 수정
#define FILENAME "dataset_id_ascending.csv"

// 퀵 정렬 최적화 임계값 (이 크기 이하일 때 삽입 정렬 사용)
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

// 함수 프로토타입 선언
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

// 중복 검사용 비교 함수 (카운트 증가 안함)
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

// qsort용 래퍼 함수들
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
        // 여기서는 카운트 증가시키지 않는 순수 비교만 수행
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
        // system("pause"); // 필요 시 주석 해제
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

            // 1. Shell Sort
            long long comp = 0;
            for (int k = 0; k < ITERATIONS; k++) {
                copy_array(buffer, data, count);
                comparison_count = 0;
                shell_sort_improved(buffer, count, criteria[c], asc);
                if (k == 0) comp = comparison_count;
            }
            // Shell Sort는 추가 메모리 거의 없음
            printf("%-20s %18lld %16d\n", "Shell Sort", comp, 0);

            // 2. Quick Sort
            comp = 0;
            for (int k = 0; k < ITERATIONS; k++) {
                copy_array(buffer, data, count);
                comparison_count = 0;
                quick_sort_improved(buffer, 0, count - 1, criteria[c], asc);
                if (k == 0) comp = comparison_count;
            }
            // Quick Sort는 재귀 스택 메모리 정도만 사용
            printf("%-20s %18lld %16d\n", "Quick Sort", comp, 0);

            // 3. Tree Sort (AVL)
            // 중복 데이터가 있으면 트리는 정렬 용도로 부적합할 수 있음(덮어쓰기 등)
            // 과제 요구사항에 따라 중복 시 SKIP
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
    // system("pause");
    return 0;
}

void copy_array(Student* dest, Student* src, int count) {
    memcpy(dest, src, sizeof(Student) * count);
}

void swap(Student* a, Student* b) {
    Student temp = *a; *a = *b; *b = temp;
}

// 비교 함수: 호출될 때마다 comparison_count 증가
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

// ==================== SHELL SORT (Ciura Sequence) ====================
// 비교 횟수가 매우 적은 것으로 알려진 Ciura 간격 수열 사용
void shell_sort_improved(Student* arr, int n, Criteria c, int asc) {
    int gaps[] = { 701, 301, 132, 57, 23, 10, 4, 1 };
    int ng = 8;

    for (int g = 0; g < ng; g++) {
        int gap = gaps[g];
        if (gap >= n) continue; // 배열 크기보다 큰 간격은 패스

        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            int j;
            // 내부 루프: 조건 검사에서 compare_students 호출됨
            for (j = i; j >= gap && compare_students(arr[j - gap], temp, c, asc) > 0; j -= gap) {
                arr[j] = arr[j - gap];
            }
            arr[j] = temp;
        }
    }
}

// ==================== QUICK SORT (Median-of-3 + Insertion Cutoff) ====================
// 1. 작은 구간에 대해 삽입 정렬 사용 (재귀 오버헤드 및 비교 감소)
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

// 2. Median-of-Three 피봇 선정으로 최악의 경우 방지
void quick_sort_improved(Student* arr, int low, int high, Criteria c, int asc) {
    if (low >= high) return;

    // 개선점 1: 작은 배열은 삽입 정렬로 처리 (비교 횟수 대폭 감소)
    if (high - low <= INSERTION_THRESHOLD) {
        insertion_sort_for_quick(arr, low, high, c, asc);
        return;
    }

    // 개선점 2: Median-of-Three 피봇 선정
    int mid = low + (high - low) / 2;

    // 피봇 정렬 (low, mid, high 세 값을 정렬하여 중앙값을 low 위치나 high-1 위치로 보냄)
    // 여기서는 간단히 low, mid, high를 비교하여 정렬해둠 (약간의 비교가 추가되지만 전체 분할 효율 상승)
    if (compare_students(arr[mid], arr[low], c, asc) < 0) swap(&arr[low], &arr[mid]);
    if (compare_students(arr[high], arr[low], c, asc) < 0) swap(&arr[low], &arr[high]);
    if (compare_students(arr[high], arr[mid], c, asc) < 0) swap(&arr[mid], &arr[high]);

    // 이제 arr[mid]가 중앙값임. 피봇으로 사용하기 위해 high-1로 숨김 (Lomuto/Hoare 변형)
    // 혹은 단순히 Hoare 파티셔닝 사용. 여기서는 Hoare 파티셔닝 사용.
    Student pivot = arr[mid];

    // Hoare Partition
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

    // 재귀 호출
    if (low < j) quick_sort_improved(arr, low, j, c, asc);
    if (i < high) quick_sort_improved(arr, i, high, c, asc);
}

// ==================== TREE SORT (AVL) ====================
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

    // 비교 결과를 변수에 저장하여 중복 호출 방지
    int cmp = compare_students(data, node->data, c, asc);

    if (cmp < 0) node->left = insert_avl(node->left, data, c, asc);
    else if (cmp > 0) node->right = insert_avl(node->right, data, c, asc);
    else return node; // 중복 값은 허용 안 하거나 무시 (과제 특성상)

    node->height = 1 + max_int(get_height(node->left), get_height(node->right));
    int bal = get_balance(node);

    // Balance Check: 비교 시 이미 node->left/right->data에 접근하므로 여기서도 compare_students 사용
    // 주의: node->left가 NULL이 아닌지 확인 필요하지만, bal > 1 이면 left는 존재함
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