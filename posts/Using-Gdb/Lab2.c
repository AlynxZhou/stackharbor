#include <stdio.h>
#include <stdlib.h>
#include <time.h>
int main(int argc, char *argv[]) {
//程序片段开始

    //题干描述
    printf("[实验2说明]本练习运行一个三重for循环, 循环结构如下:\n");
    printf("int count = 0;\n");
    printf("for(int i=0; j<LoopEnd1; i++)\n");
    printf("{\n");
    printf("\t//第一重循环体\n");
    printf("\tfor(int j=0; j<LoopEnd2; j++)\n");
    printf("\t{\n");
    printf("\t\t//第二重循环体\n");
    printf("\t\tfor(int s=0; s<LoopEnd3; s++)\n");
    printf("\t\t{\n");
    printf("\t\t\tcount++;//第三重循环体\n");
    printf("\t\t}\n");
    printf("\t}\n");
    printf("}\n");

    int LoopEnd1 = 0, LoopEnd2 = 0, LoopEnd3 = 0;

    //获取LoopEnd1的值
    printf("现在请你输入LoopEnd1的值（10~50之间）：");
    scanf("%d", &LoopEnd1);
    while(LoopEnd1<10 || LoopEnd1>50)
    {
        printf("输入错误，请重新输入LoopEnd1的值（10~50之间）：");
        scanf("%d", &LoopEnd1);
    }

     //获取LoopEnd2的值
    printf("现在请你输入LoopEnd2的值（10~50之间）：");
    scanf("%d", &LoopEnd2);
    while(LoopEnd2<10 || LoopEnd2>50)
    {
        printf("输入错误，请重新输入LoopEnd2的值（10~50之间）：");
        scanf("%d", &LoopEnd2);
    }

    //获取LoopEnd3的值
    printf("现在请你输入LoopEnd3的值（10~50之间）：");
    scanf("%d", &LoopEnd3);
    while(LoopEnd3<10 || LoopEnd3>50)
    {
        printf("输入错误，请重新输入LoopEnd3的值（10~50之间）：");
        scanf("%d", &LoopEnd3);
    }

    printf("\n你输入的三个循环变量终止值分别是：\n\n");
    printf("      LoopEnd1 = %d ; LoopEnd2 = %d ; LoopEnd3 = %d ;\n", LoopEnd1, LoopEnd2, LoopEnd3);

    int ask1, ask2, ask3, ask4;
    printf("请问第三重循环（语句count++;）最终的值是：");
    scanf("%d", &ask4);

    //三重循环分别的随机执行次数
    int tmp1, tmp2, tmp3;

	//初始化随机函数发生器
    srand((unsigned)time(NULL));

	//生成三重循环的随机数
    tmp1 = rand() % (LoopEnd1*LoopEnd2*LoopEnd3) + 1;
    tmp2 = rand() % (LoopEnd1*LoopEnd2*LoopEnd3) + 1;
    tmp3 = rand() % (LoopEnd1*LoopEnd2*LoopEnd3) + 1;

	//打印问题并获取用户答案
    printf("请问当语句“count++;”执行了%d次后，i的值为：", tmp1);
    scanf("%d", &ask1);
    printf("请问当语句“count++;”执行了%d次后，j的值为：", tmp2);
    scanf("%d", &ask2);
    printf("请问当语句“count++;”执行了%d次后，s的值为：", tmp3);
    scanf("%d", &ask3);

	//count计数的结果
    int result1 = 0, result2 = 0, result3 = 0;

    int i, j, s, count = 0;
    for(i=0; i<LoopEnd1; i++)
    {
        for(j=0; j<LoopEnd2; j++)
        {
            for(s=0; s<LoopEnd3; s++)
            {
                count++;
                if(tmp1 == count)
				{
                    result1 = i;
				}
				if(tmp2 == count)
				{
					result2 = j;
				}
				if(tmp3 == count)
				{
					result3 = s;
				}
            }
        }
    }

    printf("\n\n正确答案是：\n");
    printf("请问第三重循环（语句count++;）最终的值是：%d\n",count);
    printf("请问当第一重循环体执行了%d次后，i的值为：%d\n", tmp1,result1);
    printf("请问当第二重循环体执行了%d次后，j的值为：%d\n", tmp2, result2);
    printf("请问当第三重循环体执行了%d次后，s的值为：%d\n\n\n", tmp3, result3);

	//重新初始化随机函数发生器
    srand((unsigned)time(NULL));

	//重新生成三重循环的随机数
    tmp1 = rand() % LoopEnd1;
    tmp2 = rand() % (LoopEnd1*LoopEnd2) + 1;
    tmp3 = rand() % (LoopEnd1*LoopEnd2*LoopEnd3) + 1;

	//打印问题并获取用户答案
    printf("请问当第一重循环体执行了%d次后，count的值为：", tmp1);
    scanf("%d", &ask1);
    printf("请问当第二重循环体执行了%d次后，count的值为：", tmp2);
    scanf("%d", &ask2);
    printf("请问当第三重循环体执行了%d次后，count的值为：", tmp3);
    scanf("%d", &ask3);

	count = 0;
    for(i=0; i<LoopEnd1; i++)
    {
        for(j=0; j<LoopEnd2; j++)
        {
            for(s=0; s<LoopEnd3; s++)
            {
                count++;
                if(tmp3 ==  i*LoopEnd2*LoopEnd3 + j*LoopEnd3 + s + 1)
				{
                    result3 = count;
				}
            }
            if(tmp2 == i*LoopEnd2 + j+1)
			{
                result2 = count;
			}
        }
        if(tmp1 == i+1)
		{
            result1 = count;
		}
    }

    printf("\n正确答案是：\n");
    printf("请问当第一重循环体执行了%d次后，count的值为：%d\n", tmp1,result1);
    printf("请问当第二重循环体执行了%d次后，count的值为：%d\n", tmp2, result2);
    printf("请问当第三重循环体执行了%d次后，count的值为：%d\n", tmp3, result3);

 //程序片段结束
 return 0;
}
