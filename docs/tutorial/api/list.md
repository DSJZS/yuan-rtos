# 数据结构

## 链表

Yuan RTOS 中的链表采用双向循环链表实现，链表头本身也是一个链表节点。

- 特点
  - 任意节点都知道前驱和后继，适合频繁插入和删除
  - 头节点不存放业务数据，主要用于标识一个链表
  - 空链表状态下，头节点的 `prev` 和 `next` 都指向自己
  - 可作为调度队列、阻塞队列、定时器链表等基础容器
- 适用场景
  - 任务就绪队列
  - IPC 阻塞队列
  - 软件定时器有序链表
  - 其它需要 O(1) 插入/删除的内核对象组织方式

### 初始化链表

```c
void yr_list_init( yr_list_t* list);
```

用于初始化作为链表头的链表节点，其余链表节点可以不使用该函数进行初始化。

- 参数
  - list 链表节点指针

### 判断链表是否为空

```c
yr_bool_t yr_list_isempty( yr_list_t* list);
```

用于判断链表是否为空。

- 参数
  - list 作为链表头的链表节点的指针(其余节点传入无意义)

### 链表节点前插节点

```c
void yr_list_insert_before( yr_list_t* list, yr_list_t* node);
```

在一个指定的链表节点前插入链表节点。

- 参数
  - list 链表节点指针
  - node 要插入的链表节点的指针

### 链表节点后插节点

```c
void yr_list_insert_after( yr_list_t* list, yr_list_t* node);
```

在一个指定的链表节点后插入链表节点。

- 参数
  - list 链表节点指针
  - node 要插入的链表节点的指针

### 链表节点前删节点

```c
void yr_list_delete_before( yr_list_t* node);
```

删除一个指定的链表节点前的链表节点。

- 参数
  - node 链表节点指针

### 链表节点后删节点

```c
void yr_list_delete_after( yr_list_t* node);
```

删除一个指定的链表节点后的链表节点。

- 参数
  - node 链表节点指针

### 链表节点自删除

```c
void yr_list_delete_self( yr_list_t* node);
```

将一个节点从其锁在的链表删除。

- 参数
  - node 链表节点指针

### 链表示例

```c
typedef struct demo_node_t {
    int value;
    yr_list_t list_node;
} demo_node_t;

static yr_list_head_t demo_list;
static demo_node_t node1 = { .value = 1 };
static demo_node_t node2 = { .value = 2 };

void demo_list_init(void)
{
    yr_list_init(&demo_list);

    yr_list_insert_before(&demo_list, &node1.list_node);
    yr_list_insert_before(&demo_list, &node2.list_node);
}
```

