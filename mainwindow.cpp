#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSoundEffect>
#include "Item.h"
int chsXstart = 280, chsXend = 760;
int chsYstart = 40, chsYend = 520;
int lDist = 60;
bool visited_by_air_judge[9][9] = { {false} };
const int dx[4] = { -1, 0, 1, 0 };
const int dy[4] = { 0, -1, 0, 1 };
bool flag=1;
bool buttonFlage=1;
bool tempstart = 0;
QLabel*loss_label1;
QLabel*loss_label2;
int btn1=1;
int btn2=1;
int btn3=1;
int f=0;
#define MAXBranchNum 81


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(0, 0, 800, 560);

    resize(800, 560);
    setWindowTitle("NoGo");

    QPalette pa(this->palette());
    QImage img=QImage(":/bg_c");
    img=img.scaled(this->size());
    QBrush*pic=new QBrush(img);
    pa.setBrush(QPalette::Window,*pic);
    this->setPalette(pa);

    QFont f1;
    f1.setFamily("Cascadia Code");
    f1.setPixelSize(115);
    label1=new QLabel(this);
    label1->setText("NoGo");
    label1->move(0,40);
    label1->resize(300,150);
    label1->setFont(f1);
    label1->setScaledContents(true);
    label1->show();

    f=1;

    gameButton = new QPushButton("新建", this);
    exitButton = new QPushButton("退出", this);
    gameButton->setGeometry(40,225,190,60);
    exitButton->setGeometry(40,300,190,60);
    gameButton->setFont(QFont("黑体",30));
    exitButton->setFont(QFont("黑体",30));
    connect(this->gameButton, SIGNAL(clicked()), this, SLOT(gameButton_Clicked()));
    connect(this->exitButton, SIGNAL(clicked()), this, SLOT(exitButton_Clicked()));


    pushButton1 = new QPushButton("先手", this);
    pushButton2 = new QPushButton("后手", this);
    pushButton3 = new QPushButton("读档", this);
    pushButton1->hide();
    pushButton2->hide();
    pushButton3->hide();
    pushButton1->setGeometry(40,225,190,60);
    pushButton2->setGeometry(40,300,190,60);
    pushButton3->setGeometry(40,375,190,60);
    pushButton1->setFont(QFont("黑体",30));
    pushButton2->setFont(QFont("黑体",30));
    pushButton3->setFont(QFont("黑体",30));
    connect(this->pushButton1, SIGNAL(clicked()), this, SLOT(pushButton1_Clicked()));
    connect(this->pushButton2, SIGNAL(clicked()), this, SLOT(pushButton2_Clicked()));
    connect(this->pushButton3, SIGNAL(clicked()), this,SLOT(pushButton3_Clicked()));

    m_bIsBlackTun=0;
    m_items.clear();
    memset(chessboard,0,sizeof(chessboard));
    firsthand=0;
}

MainWindow::~MainWindow()
{
}

void MainWindow::gameButton_Clicked()
{

    pushButton1->show();
    pushButton2->show();
    pushButton3->show();

    f=2;
}
void MainWindow::exitButton_Clicked()
{
    exit(0);
}
void MainWindow::pushButton1_Clicked() {

    if(btn1==1)
    {
        buttonFlage=0;
        firsthand=1;
        pushButton1->setText("悔棋");
        pushButton2->setText("存档");
        pushButton3->setText("返回");
        btn1=2;btn2=2;btn3=2;
    }
    else
    {
        for(int i=0;i<2;i++){
            Item item;
            if(m_items.size()==0)
                return;
            item=m_items.back();
            chessboard[item.m_pt.x()][item.m_pt.y()]=0;
            m_items.pop_back();
        }
    }

}
void MainWindow::pushButton2_Clicked() {

    if(btn2==1)
    {    buttonFlage=0;
        firsthand=0;
        if(!firsthand&&flag)
        {
            flag=0;
            mcts();
        }
        pushButton1->setText("悔棋");
        pushButton2->setText("存档");
        pushButton3->setText("返回");
        btn1=2;btn2=2;btn3=2;
        return;
    }
    else
    {
        save();
    }
}
void MainWindow::pushButton3_Clicked() {
    if(btn3==1)
    {
        load();
        buttonFlage=0;
        pushButton1->setText("悔棋");
        pushButton2->setText("存档");
        pushButton3->setText("返回");
        btn1=2;btn2=2;btn3=2;
        return;
    }
    else
    {
        exit(0);
    }
}
void MainWindow::DrawItem()
{

    QPainter painter(this);
    QPen pen;
    pen.setColor(Qt::transparent);
    painter.setPen(pen);
    for(int i=0;i<m_items.size();i++)
    {
        Item item=m_items[i];

        if(!item.m_bBlack)
            painter.setBrush(Qt::black);
        else
            painter.setBrush(Qt::white);

        QPoint ptCenter((item.m_pt.x()*lDist)+280,(item.m_pt.y()*lDist)+40);
        painter.drawEllipse(ptCenter,lDist/2,lDist/2);
    }
}
bool inBoard_judge(int x, int y)
{
    return 0 <= x && x < 9 && 0 <= y && y < 9;
}
bool air_judge(int board[9][9], int x, int y)
{
    visited_by_air_judge[x][y] = true; // 标记，表示这个位置已经搜过有无气了
    bool flag = false;
    for (int dir = 0; dir < 4; ++dir)
    {
        int x_dx = x + dx[dir], y_dy = y + dy[dir];
        if (inBoard_judge(x_dx, y_dy)) // 界内
        {
            if (board[x_dx][y_dy] == 0) // 旁边这个位置没有棋子
                flag = true;
            if (board[x_dx][y_dy] == board[x][y] && !visited_by_air_judge[x_dx][y_dy]) // 旁边这个位置是没被搜索过的同色棋
                if (air_judge(board, x_dx, y_dy))
                    flag = true;
        }
    }
    return flag;
}
bool put_available(int board[9][9], int x, int y, int color)
{
    if (!inBoard_judge(x, y))
        return false;
    if (board[x][y]) // 如果这个点本来就有棋子
        return false;

    board[x][y] = color;
    memset(visited_by_air_judge, 0, sizeof(visited_by_air_judge)); // 重置

    if (!air_judge(board, x, y)) // 如果下完这步这个点没气了,说明是自杀步，不能下
    {
        board[x][y] = 0;
        return false;
    }

    for (int i = 0; i < 4; ++i) // 判断下完这步周围位置的棋子是否有气
    {
        int x_dx = x + dx[i], y_dy = y + dy[i];
        if (inBoard_judge(x_dx, y_dy)) // 在棋盘内
        {
            if (board[x_dx][y_dy] && !visited_by_air_judge[x_dx][y_dy]) // 对于有棋子的位置（标记访问过避免死循环）
                if (!air_judge(board, x_dx, y_dy))                      // 如果导致(x_dx,y_dy)没气了，则不能下
                {
                    board[x][y] = 0; // 回溯
                    return false;
                }
        }
    }
    board[x][y] = 0; // 回溯
    return true;
}
int getValidPositions(int board[9][9], int result[9][9])
{
    memset(result, 0, MAXBranchNum * 4);
    int right = 0;
    for (int x = 0; x < 9; ++x)
    {
        for (int y = 0; y < 9; ++y)
        {
            if (put_available(board, x, y, 1))
            {
                right++;
                result[x][y] = 1;
            }
        }
    }
    return right;
}
class MainWindow::treeNode
{
public:
    treeNode* parent;                 // 父节点
    treeNode* children[MAXBranchNum]; // 子节点
    int board[9][9];
    int childrenAction[MAXBranchNum][2];
    int childrenCount;
    int childrenCountMax;
    double value;      // 该节点的总value
    int n;             // 当前节点探索次数，UCB中的ni
    double UCB;        // 当前节点的UCB值
    int* countPointer; // 总节点数的指针
    // 构造函数
    treeNode(int parentBoard[9][9], int opp_action[2], treeNode* parentPointer, int* countp) // 构造函数 treeNode *p是父类指针, int *countp应该是总探索次数的指针
    {
        for (int i = 0; i < 9; ++i) // 把棋盘反过来，要落子方是1 ，对手是-1
        {
            for (int j = 0; j < 9; ++j)
            {
                board[i][j] = -parentBoard[i][j];
            }
        }
        if (opp_action[0] >= 0 && opp_action[0] < 9 && opp_action[1] >= 0 && opp_action[1] < 9)
            board[opp_action[0]][opp_action[1]] = -1;
        parent = parentPointer;
        value = 0;
        n = 0;
        childrenCount = 0;     // 已经拓展的子节点数
        countPointer = countp; // count的指针
        evaluate();            // 计算能下的位置,修改了childrenCountMax、childrenAction
    }
    treeNode* treeRules() // 搜索法则
    {
        // 如果没有位置下了（终局）
        if (childrenCountMax == 0)
        {
            return this; // 到达终局当前叶节点
        }

        // 如果是叶节点，Node Expansion，拓展下一层节点
        if (childrenCountMax > childrenCount)
        {
            treeNode* newNode = new treeNode(board, childrenAction[childrenCount], this, countPointer); // 拓展一个子节点
            children[childrenCount] = newNode;
            childrenCount++; // 已拓展的子节点数++
            return newNode;
        }

        // 计算当前节点的每个子节点的UCB值（点亮某个节点）
        for (int i = 0; i < childrenCount; ++i)
        {
            children[i]->UCB = children[i]->value / double(children[i]->n) + 0.2 * sqrt(log(double(*countPointer)) / double(children[i]->n)); // UCB公式
        }
        int bestChild = 0;
        double maxUCB = 0;

        // 找出所有子节点中UCB值最大的子节点
        for (int i = 0; i < childrenCount; ++i)
        {
            if (maxUCB < children[i]->UCB)
            {
                bestChild = i;
                maxUCB = children[i]->UCB;
            }
        }
        return children[bestChild]->treeRules(); // 对UCB最大的子节点进行下一层搜索
    }
    // 模拟

    double simulation()
    {
        int board_opp[9][9]; // 对手棋盘
        int res[9][9];
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                board_opp[i][j] = -board[i][j];
            }
        }
        int x = getValidPositions(board, res);    // 落子方可下位置数
        int y = getValidPositions(board_opp, res); // 非落子方可下位置数
        return x - y;
    }
    void backup(double deltaValue) // 回传估值,从当前叶节点以及往上的每一个父节点都加上估值
    {
        treeNode* node = this;
        int side = 0;
        while (node != nullptr) // 当node不是根节点的父节点时
        {
            if (side == 1) // 落子方
            {
                node->value += deltaValue;
                side--;
            }
            else // 非落子方
            {
                node->value -= deltaValue;
                side++;
            }
            node->n++; // 当前节点被探索次数++
            node = node->parent;
        }
    }

private:
    void evaluate() // 计算能下的位置,修改了childrenCountMax、childrenAction
    {
        int result[9][9];
        int validPositionCount = getValidPositions(board, result); // 能下的位置数
        int validPositions[MAXBranchNum];                          // 能下的位置坐标
        int availableNum = 0;
        for (int i = 0; i < 9; ++i)
        {
            for (int j = 0; j < 9; ++j)
            {
                if (result[i][j])
                {
                    validPositions[availableNum] = i * 9 + j; // 可下的位置
                    availableNum++;                           // 可下的位置数
                }
            }
        }
        childrenCountMax = validPositionCount; // 总共能下的位置数
        for (int i = 0; i < validPositionCount; ++i)
        {
            childrenAction[i][0] = validPositions[i] / 9;
            childrenAction[i][1] = validPositions[i] % 9;
        }
    }
};
void MainWindow::mcts()
{
    int count = 0; // 总计算的节点数（总探索次数，UCB中的N）
    int board[9][9] = { {0} };
    srand(clock());
    int start = clock();
    int timeout = (int)(0.98 * (double)CLOCKS_PER_SEC);
    memcpy(board, chessboard, 324);
    int x = tempbackXUser, y = tempbackYUser;
    int opp_action[2] = { x, y }; // 对面上一步走了哪里

    treeNode rootNode(board, opp_action, nullptr, &count); // 创建根节点，根节点的父节点为空

    while (clock() - start < timeout)
    {
        count++;                               // 计算的节点数++
        treeNode* node = rootNode.treeRules(); // 拓展一次，node指向的是一次拓展的叶节点
        double result = node->simulation();    // 结果估值
        node->backup(result);
    }

    int bestChildren[MAXBranchNum] = { 0 }; // 所有最优子节点的序号
    int bestChildrenNum = 0;              // 最优子节点个数
    int maxValue = INT_MIN;               // 当前最优子节点分数
    for (int i = 0; i < rootNode.childrenCount; ++i)
    {
        if (maxValue < rootNode.children[i]->value)
        {
            // 重置
            memset(bestChildren, 0, sizeof(bestChildren));
            bestChildrenNum = 0;

            bestChildren[bestChildrenNum++] = i;
            maxValue = rootNode.children[i]->value;
        }
        else if (maxValue == rootNode.children[i]->value)
        {
            bestChildren[bestChildrenNum++] = i;
        }
    }

    if (!bestChildrenNum)
    {
        overi = -1;
        Failedback();
    }
    int random = rand() % bestChildrenNum;                           // 在所有最优中任选一个
    int* bestAction = rootNode.childrenAction[bestChildren[random]]; // 最优子节点对应走法
    int tempx = bestAction[0];
    int tempy = bestAction[1];

    //撤回
    tempbackXAI = tempx;
    tempbackYAI = tempy;

    QPoint pt2;
    pt2.setX(tempx);
    pt2.setY(tempy);
    Item item2(pt2,m_bIsBlackTun);
    m_items.append(item2);
    m_bIsBlackTun=!m_bIsBlackTun;
    tempstart = 1;
    chessboard[tempx][tempy] = -1;
}
void MainWindow::Failedback()
{
    label1->hide();
    pushButton1->hide();
    pushButton2->hide();
    pushButton3->hide();
    gameButton->hide();
    exitButton->hide();
    buttonFlage=1;
    loss_label1=new QLabel(this);
    loss_label2=new QLabel(this);
    loss_label1->move(280,200);
    loss_label2->move(280,260);
    loss_label1->resize(200,200);
    loss_label2->resize(400,200);
    QFont f;
    f.setPixelSize(50);
    f.setFamily("黑体");
    loss_label1->setFont(f);
    loss_label2->setFont(f);

    if(overi!=0)
    {
        loss_label1->setText(over[overi+1]);
        loss_label2->setText("输了!");
        if(overi==1)
        {
            loss_label1->setStyleSheet("color:red;");
            loss_label2->setStyleSheet("color:red;");
        }
        else if(overi==-1)
        {
            loss_label1->setStyleSheet("color:green;");
            loss_label2->setStyleSheet("color:green;");
        }

        loss_label1->show();
        loss_label2->show();
        repaint();

        QThread::msleep(2000);
    }
    exitButton->setGeometry(300,450,200,60);
    QPalette pa(this->palette());
    QImage img=QImage(":/w");
    img=img.scaled(this->size());
    QBrush*pic=new QBrush(img);
    pa.setBrush(QPalette::Window,*pic);
    this->setPalette(pa);
    m_items.clear();
    exitButton->show();
}

void MainWindow::mousePressEvent(QMouseEvent*event)
{
    if(buttonFlage)
        return;

    QPoint pt1;
    int chess_x=event->pos().x();
    int chess_y=event->pos().y();
    if(chess_x<=250)
        return;

    int tmpx=chess_x-280;
    int tmpy=chess_y-40;
    for(int i=0;i<=8;i++)
    {
        if(tmpx<=30*(2*i+1))
        {
            tmpx=60*i;
            break;
        }
    }
    for(int j=0;j<=8;j++)
    {
        if(tmpy<=30*(2*j+1))
        {
            tmpy=60*j;
            break;
        }
    }
    pt1.setX(tmpx/lDist);
    pt1.setY(tmpy/lDist);

    if (tempstart == 0 && pt1.x() == 4 && pt1.y() == 4)
    {
        Failedback();
        loss_label1->setText("禁止首着");
        loss_label2->setText("落于正中心");
        loss_label1->show();
        loss_label2->show();
        return;
    }
    tempstart = 1;

    if (!chessboard[pt1.x()][pt1.y()])
    {
        if (!put_available(chessboard, pt1.x(), pt1.y(), 1))
        {
            overi = 1;
            Failedback();
        }
        else
        {
            Item item1(pt1,m_bIsBlackTun);
            m_items.append(item1);
            m_bIsBlackTun=!m_bIsBlackTun;
            chessboard[pt1.x()][pt1.y()] = 1;
            mcts();
        }
        return;
    }
}
void MainWindow::save()
{
    QFile outfile(QApplication::applicationDirPath() + "/savefile.txt");

    if(outfile.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        QTextStream out(&outfile);
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                out << chessboard[j][i]<<" ";
            }
            out<<"\n";
        }
        out << firsthand<<"\n";
        out<<m_items.size()<<"\n";
        for(int k=0;k<m_items.size();k++)
        {
            out << m_items[k].m_pt.x()<<" "<< m_items[k].m_pt.y()<<" "<<m_items[k].m_bBlack<<"\n";
        }
        out<<tempstart;
        outfile.close();
        qDebug()<<"completed.";
    }
    else
    {
        qDebug()<<"failed to create a new file!";
    }
}
void MainWindow::load()
{
    QFile infile("savefile.txt");
    if (!infile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug()<<"failed";
        Failedback();
        loss_label1->setText("读取失败");
        loss_label1->show();
    }
    else
    {
        qDebug()<<"completed.";
    }

    int _size;
    QPoint _pt;
    int tmpx,tmpy;
    int bb;
    QTextStream in(&infile);

        for (int i = 0; i < 9; i++)
        {   for (int j = 0; j < 9; j++)
            {
                in >> chessboard[j][i];
            }
        }
        in >> firsthand;
        in>>_size;
        for(int t=0;t<_size;t++)
        {
            in>>tmpx>>tmpy>>bb;
            _pt.setX(tmpx);
            _pt.setY(tmpy);
            Item _item(_pt,bb);
            m_items.append(_item);
        }
        in >> tempstart;
    infile.close();
}


void MainWindow::paintEvent(QPaintEvent*event)
{
    DrawItem();
    update();
}
