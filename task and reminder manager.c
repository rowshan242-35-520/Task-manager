#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_TASKS 100
#define MAX_TITLE 50
#define MAX_DESC 100
#define QUEUE_SIZE 50
#define STACK_SIZE 100

struct Task {
    char title[MAX_TITLE];
    char desc[MAX_DESC];
    int priority;
    char deadline[6];
};

struct Action {
    int type;
    struct Task task;
    int index;
};

struct Stack {
    struct Action arr[STACK_SIZE];
    int top;
};

void initializeStack(struct Stack *stack) {
    stack->top = -1;
}
bool isStackEmpty(struct Stack *stack) {
    return stack->top == -1;
}
bool isStackFull(struct Stack *stack) {
    return stack->top == STACK_SIZE - 1;
}
void push(struct Stack *stack, struct Action value) {
    if (isStackFull(stack)) {
        printf("Stack Overflow\n");
        return;
    }
    stack->arr[++stack->top] = value;
}
struct Action pop(struct Stack *stack) {
    struct Action a = {0};
    if (isStackEmpty(stack)) {
        printf("Stack Underflow\n");
        return a;
    }
    a = stack->arr[stack->top--];
    return a;
}
struct Action peek(struct Stack *stack) {
    struct Action a = {0};
    if (isStackEmpty(stack)) {
        printf("Stack is empty\n");
        return a;
    }
    return stack->arr[stack->top];
}

struct ActionQueue {
    struct Action arr[QUEUE_SIZE];
    int front;
    int back;
};

void initializeActionQueue(struct ActionQueue *q) {
    q->front = 0;
    q->back = -1;
}
bool isActionQueueEmpty(struct ActionQueue *q) {
    return q->front > q->back;
}
bool isActionQueueFull(struct ActionQueue *q) {
    return q->back - q->front + 1 >= QUEUE_SIZE;
}
void enqueueAction(struct ActionQueue *q, struct Action value) {
    if (isActionQueueFull(q)) {
        q->front++;
    }
    q->arr[++q->back] = value;
}

struct Task tasks[MAX_TASKS];
int taskCount = 0;

struct Stack actionStack;

struct ActionQueue actionHistoryQueue;

void saveTasksToFile() {
    FILE *f = fopen("tasks.txt", "w");
    if (f) {
        fprintf(f, "%d\n", taskCount);
        for (int i = 0; i < taskCount; i++) {
            fprintf(f, "%s\n%s\n%d\n%s\n", tasks[i].title, tasks[i].desc, tasks[i].priority, tasks[i].deadline);
        }
        fclose(f);
    }
}
void loadTasksFromFile() {
    FILE *f = fopen("tasks.txt", "r");
    if (f) {
        fscanf(f, "%d\n", &taskCount);
        for (int i = 0; i < taskCount; i++) {
            fgets(tasks[i].title, MAX_TITLE, f);
            tasks[i].title[strcspn(tasks[i].title, "\n")] = 0;
            fgets(tasks[i].desc, MAX_DESC, f);
            tasks[i].desc[strcspn(tasks[i].desc, "\n")] = 0;
            fscanf(f, "%d\n", &tasks[i].priority);
            fgets(tasks[i].deadline, 6, f);
            tasks[i].deadline[strcspn(tasks[i].deadline, "\n")] = 0;
        }
        fclose(f);
    }
}
void removeTaskFromFile(int idx) {
    for (int i = idx; i < taskCount - 1; i++) {
        tasks[i] = tasks[i + 1];
    }
    taskCount--;
    saveTasksToFile();

    struct Action a = {3, tasks[idx], idx};
    enqueueAction(&actionHistoryQueue, a);
}

void addTask() {
    if (taskCount >= MAX_TASKS) {
        printf("Task list full!\n");
        return;
    }
    struct Task t;
    printf("Enter title: ");
    fgets(t.title, MAX_TITLE, stdin);
    t.title[strcspn(t.title, "\n")] = 0;
    printf("Enter description: ");
    fgets(t.desc, MAX_DESC, stdin);
    t.desc[strcspn(t.desc, "\n")] = 0;
    printf("Enter priority (1=High,2=Med,3=Low): ");
    scanf("%d", &t.priority);
    getchar();
    printf("Enter deadline (DD-MM): ");
    fgets(t.deadline, 6, stdin);
    t.deadline[strcspn(t.deadline, "\n")] = 0;
    tasks[taskCount] = t;
    struct Action a = {1, t, taskCount};
    push(&actionStack, a);// ln 39
    enqueueAction(&actionHistoryQueue, a);
    taskCount++;
    saveTasksToFile();
    printf("Task added!\n");
}

void viewTasks() {
    printf("\n%-3s %-20s %-10s %-7s %-10s\n", "ID", "Title", "Priority", "Deadline", "Desc");
    for (int i = 0; i < taskCount; i++) {
        printf("%-3d %-20s %-10d %-7s %-10s\n", i+1, tasks[i].title, tasks[i].priority, tasks[i].deadline, tasks[i].desc);
    }
}

void sortTasks(int byPriority) {
    for (int i = 0; i < taskCount-1; i++) {
        for (int j = 0; j < taskCount-i-1; j++) {
            int cmp = 0;
            if (byPriority)
                cmp = tasks[j].priority > tasks[j+1].priority;
            else
                cmp = strcmp(tasks[j].deadline, tasks[j+1].deadline) > 0;
            if (cmp) {
                struct Task tmp = tasks[j];
                tasks[j] = tasks[j+1];
                tasks[j+1] = tmp;
            }
        }
    }
    printf("Tasks sorted!\n");

    struct Action a = {2, tasks[0], 0};
    enqueueAction(&actionHistoryQueue, a);
}

void undoLastAction() {
    if (isStackEmpty(&actionStack)) {
        printf("Nothing to undo!\n");
        return;
    }
    struct Action a = pop(&actionStack);// ln 55
    if (a.type == 1) {
        if (a.index == taskCount - 1) {
            removeTaskFromFile(a.index);
            printf("Last added task undone.\n");
        }
    } else if (a.type == 2) {
    }
}

void showActionHistory() {
    printf("\n%-10s %-6s %-20s\n", "Action", "ID", "Title");
    for (int i = actionHistoryQueue.front; i <= actionHistoryQueue.back; i++) {
        struct Action *a = &actionHistoryQueue.arr[i];
        char *typeStr;
        switch (a->type) {
            case 1: typeStr = "Added"; break;
            case 2: typeStr = "Sorted"; break;
            case 3: typeStr = "Removed"; break;
            case 4: typeStr = "UndoAdd"; break;
            default: typeStr = "Other";
        }
        printf("%-10s %-6d %-20s\n", typeStr, a->index + 1, a->task.title);
    }
}

int main() {
    initializeStack(&actionStack);
    initializeActionQueue(&actionHistoryQueue);
    loadTasksFromFile();
    int ch;
    while (1) {
        printf("==========Welcome to Task Manager==========\n");
        printf("\n1.Add Task\n2.View Tasks\n3.Sort by Priority\n4.Sort by Deadline\n");
        printf("5.Action History\n6.Undo Last Action\n7.Exit\n");
        printf("Enter choice: ");
        scanf("%d", &ch);
        getchar();
        switch (ch) {
            case 1: addTask(); break;
            case 2: viewTasks(); break;
            case 3: sortTasks(1); break;
            case 4: sortTasks(0); break;
            case 5: showActionHistory(); break;
            case 6: undoLastAction(); break;
            case 7: return 0;
            default: printf("Invalid choice!\n");
        }
    }
    return 0;
}