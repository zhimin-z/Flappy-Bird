# include <curses.h>
# include <stdlib.h>
# include <signal.h>
# include <sys/time.h>
# include <time.h>

const char CHAR_BIRD = 'O';  // 定义 bird 字符
const char CHAR_STONE = '*';  // 定义组成柱子的石头
const char CHAR_BLANK = ' ';  // 定义空字符

// 背景中的柱子用单向链表存储
typedef struct node {
    int x, y;
    struct node *next;
} node, *Node;

Node head, tail;
int bird_x, bird_y;
int ticker;

void init();  // 初始化函数，统筹游戏各项的初始化工作
void init_bird();  // 初始化 bird 位置坐标
void init_draw();  // 初始化背景
void init_head();  // 初始化存放柱子的链表的链表头
void init_wall();  // 初始化存放柱子的链表
void drop(int sig);  // 信号接收函数，用来接收到系统信号，从右向左移动柱子
int set_ticker(int n_msec);  // 设置内核的定时周期

void init_head()
{
    Node tmp = new node;
    tmp->next = nullptr;
    tail = head = tmp;
} 

void init_wall()
{
    Node p = head;
    for(int i = 0; i < 5; i++)
    {
        Node tmp = new node;
        tmp->x = (i + 1) * 19;
        tmp->y = rand() % 11 + 5;
        tmp->next = nullptr;
        p->next = tmp;
        p = tmp;
    }
   tail = p;
}

void init_bird()
{
    bird_x = 5;
    bird_y = 15;
    move(bird_y, bird_x);
    addch(CHAR_BIRD);
    refresh();
}

void init_draw()
{
    // 遍历链表
    for(Node p = head->next; p->next != nullptr; p = p->next)
    {
        // 绘制柱子
        for(int i = p->x; i > p->x - 10; --i) 
        {
            for(int j = 0; j < p->y; ++j) 
            {
                move(j, i);
                addch(CHAR_STONE);
            }

            for(int j = p->y + 5; j <= 23; ++j) 
            {
                move(j, i);
                addch(CHAR_STONE);
            }
        }
        refresh();
    }
}

void init()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    srand(time(0));
    signal(SIGALRM, drop);

    init_bird();
    init_head();
    init_wall();
    init_draw();
    ticker = 500;
    set_ticker(ticker);
}

// 见 linux.die.net/man/2/setitimer
int set_ticker(int n_msec)
{
    struct itimerval timeset;
    long n_sec, n_usec;

    n_sec = n_msec / 1000;
    n_usec = (n_msec % 1000) * 1000L;

    timeset.it_interval.tv_sec = n_sec;
    timeset.it_interval.tv_usec = n_usec;

    timeset.it_value.tv_sec = n_sec;
    timeset.it_value.tv_usec = n_usec;

    return setitimer(ITIMER_REAL, &timeset, nullptr);
}

void drop(int sig)
{
    // 移动 bird，并进行重绘
    move(bird_y, bird_x);
    addch(CHAR_BLANK);

    move(++bird_y, bird_x);
    addch(CHAR_BIRD);
    refresh();

    // 如果撞上柱子则结束游戏
    if((char)inch() == CHAR_STONE) 
    {
        set_ticker(0);
        endwin();
        exit(0);
    }

    // 检测第一块墙是否超出边界
    Node p = head->next;
    if(p->x < 0) 
    {
        head->next = p->next;
        delete p;

        Node tmp = new node;
        tmp->x = 99;
        tmp->y = rand() % 11 + 5;
        tmp->next = nullptr;

        tail->next = tmp;
        tail = tmp;

        ticker -= 10;  // 加速
        set_ticker(ticker);
    }

    // 绘制新的柱子
    for(p = head->next; p->next != nullptr; p->x--, p = p->next) 
    {
        // 使用 CHAR_BLANK 替代原先的 CHAR_STONE
        for(int j = 0; j < p->y; ++j) 
        {
            move(j, p->x);
            addch(CHAR_BLANK);
        }

        for(int j = p->y + 5; j <= 23; ++j) 
        {
            move(j, p->x);
            addch(CHAR_BLANK);
        }

        if(p->x >= 10 && p->x < 80) 
        {
            for(int j = 0; j < p->y; j++) 
            {
                move(j, p->x - 10);
                addch(CHAR_STONE);
            }

            for(int j = p->y + 5; j <= 23; j++) 
            {
                move(j, p->x - 10);
                addch(CHAR_STONE);
            }
        }

        refresh();
    }
    tail->x--;    
}

int main()
{
    init();
    while(true) 
    {
        char ch = getch();  // 获取键盘输入
        if(ch == ' ' || ch == 'w' || ch == 'W')  // 按下空格或者 W 时
        {
            // 移动 bird，并进行重绘
            move(bird_y, bird_x);
            addch(CHAR_BLANK);

            move(--bird_y, bird_x);
            addch(CHAR_BIRD);
            refresh();

            // 如果 bird 撞到了柱子，结束游戏
            if((char)inch() == CHAR_STONE)
            {
                set_ticker(0);
                endwin();
                exit(0);
            }
        }
        else if(ch == 'z' || ch == 'Z')  // 暂停
        {
            set_ticker(0);
            do {
                ch = getch();
            } while(ch != 'z' && ch != 'Z');
            set_ticker(ticker);
        }
        else if(ch == 'q' || ch == 'Q')  // 退出
        {
            endwin();
            exit(0);
        }
    }
    return 0;
}