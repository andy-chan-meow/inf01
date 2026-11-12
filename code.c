#include <stdio.h>

int main(){

int memory[256] = {0};
int cpu_state[8] = { 0, 0, 0, 0, 0 ,0 ,0, 0};
/*
pc - 0
acc - 1
ir - 2
break vector - 3  
stop flag - 4
error code - 5
*/

FILE *file = fopen("program.txt","r");//читаем файл
if (file == NULL) {
    printf("Error: Failed to open program.txt\n");//если файл пустой возвращаем 1
    return 1;
}

int address = 0;//начинаем считывать числа из файла
while(address < 256 && fscanf(file, "%d", &memory[address]) == 1){
    address++;
}
fclose(file);//закрываем файл

cpu_state[0] = 1;//счетчик команд
cpu_state[3] = memory[0];//записываем вектор прерывания
cpu_state[4] = 0;//нет флагов остановки
cpu_state[5] = 0;//нет ошибок

while (cpu_state[4] == 0){//выполняется пока нет стоп флагов
    if (cpu_state[0] == cpu_state[3]){//если во время работы появилась ошибка
            break;
        }
    int pc  = cpu_state[0];
    
    if(pc < 0 || pc >= 256){// проверяем не вышел ли счетчик за пределы памяти
            cpu_state[5] = 1;//ошибка адресации 
            printf("Error: Out of bounds memory access\nInterruption: Memory addressing error at address %d\n", pc);
            cpu_state[0] = cpu_state[3];//прерываем процесс
            
            continue;
        }   
        //чтение текуещй команды
    cpu_state[2] = memory[pc];
    int opcode =  cpu_state[2];

    
    if (pc + 1  >= 256) {//проверяем не вышел ли адрес ячейки операнда за пределы памяти
        printf("Error: Missing operand");
        cpu_state[5] = 1;
        cpu_state[0] = cpu_state[3];
        continue;
    }
    int operand = memory[pc + 1]; //после ячейки оператора идет ячейка операнда, поэтому адрес на 1 больше
    if(operand < 0 || operand >= 256){//проверка адреса операнда на выход за пределы памяти
                cpu_state[5] = 1;//ошибка адресации 
                cpu_state[0] = cpu_state[3];
                printf("Error: Out of bounds memory access\nInterruption: Memory addressing error at address %d\n", operand);
                continue;
            }
    int value = memory[operand];//значение ячейки-операнда
    
    switch(opcode){
        //команды, выполняемые процессором
        case 0: // остановка программы
            cpu_state[4] = 1;
            printf("pc=%d: Command 0: Stopped\n",pc);
            break;

        case 1: //загрузка значения в аккумулятор
            cpu_state[1] = value;   
            printf("pc=%d: Command 1 (Loading), operand=%d, value = %d, acc=%d, pc += 2\n", pc, operand,value, cpu_state[1]);          
            cpu_state[0] += 2 ; //переходим на 2 ячейки так как оператор + операнд занимают 2 ячейки    
            break;

        case 2: //умножение аккумулятора на значение из ячейки памяти
            cpu_state[1] *= value; 
            printf("pc=%d: Command 2 (Multiple), operand=%d, value = %d,  acc=%d, pc += 2\n", pc, operand, value, cpu_state[1]);
            cpu_state[0] += 2 ; //переходим на 2 ячейки так как оператор + операнд занимают 2 ячейки 
            break;

        case 3: //деление аккумулятора на значение из ячейки памяти
            if (value == 0){//проверка на деление на 0
                cpu_state[5] = 3;//ошибка деления на 0
                cpu_state[0] = cpu_state[3];
                printf("Interruption: Null division at the cell memory[%d], transition to %d\n", operand, cpu_state[3]);
            } else {
                cpu_state[1] /= value;
                  printf("pc=%d: Command 3 (Division), operand=%d, value = %d, acc=%d, pc += 2\n", pc, operand,value, cpu_state[1]);
                cpu_state[0] += 2 ; //переходим на 2 ячейки так как оператор + операнд занимают 2 ячейки 
            }
            break;

        case 4://JUMP
            printf("pc=%d: Command 4 (JUMP) -> come to address %d\n", pc, operand);
            cpu_state[0] = operand;
            break;

        case 5: //JUMPZ
             if (cpu_state[1] == 0){//если в аккумуляторе 0, то переходим на адрес operand 
                cpu_state[0] = operand;
            } else {//пропускаем, если не 0
                printf("pc=%d: Command 5 (JUMPZ) -> acc!=0 (%d), Ignore transition, pc += 2\n", pc, cpu_state[1]);
                cpu_state[0] += 2;
            }
            break;

        default:
            cpu_state[5] = 2;//ошибка неизвестного опкода
            cpu_state[0] = cpu_state[3];
            printf("Error: Unknown opcode %d at address %d\n", opcode, pc);
            break;
        }
    }
    //итоговое состояние процессора
    printf("Programm finished:\n");
    printf("Final pc: %d\n",cpu_state[0]);
    printf("Final acc: %d\n", cpu_state[1]);
    printf("Final error code: %d\n", cpu_state[5]);
    return 0;
}