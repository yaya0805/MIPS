#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int reg[50],pc,num,instruction[1000000],opcode,line,cycle,memory[300000],pc_in,n,flag=0;
int rs,rt,rd,shamt,immediate,address;
inline void endian_swap(unsigned int& x)
{
    x = (x>>24) |((x<<8) & 0x00FF0000) |((x>>8) & 0x0000FF00) |(x<<24);
    //printf("%08X\n",x);
}
void print(FILE *fp3){
    if(flag==0){
        fprintf(fp3,"cycle %d\n",cycle);
        //printf("cycle %d\n",cycle);
        for(int i=0;i<32;i++){
            fprintf(fp3,"$%02d: 0x%08X\n",i,reg[i]);
            //printf("$%02d: 0x%08X\n",i,reg[i]);
        }
        fprintf(fp3,"PC: 0x%08X\n\n\n",pc);
        //printf("PC: 0x%08X\n\n\n",pc);
          //          printf("%d\n",memory[7]);

        //printf("   %08X\n",line);
        //printf("   %02X\n",opcode);
        //printf("mem[255]=%08X\n",memory[255]);

        cycle++;
    }
}
void Error(int n,FILE *fp4,FILE *fp3){
    //puts("Error");
         // printf("   %08X\n",line);
    if(n==1){
        fprintf(fp4,"In cycle %d: Write $0 Error\n",cycle);
    }
    else if(n==2){
        fprintf(fp4,"In cycle %d: Number Overflow\n",cycle);
    }
    else if(n==3){
        fprintf(fp4,"In cycle %d: Address Overflow\n",cycle);
        flag=1;
    }
    else if(n==4){
        fprintf(fp4,"In cycle %d: Misalignment Error\n",cycle);
        flag=1;
    }
}
void Halt(){
    flag=1;
}
void Rtype(FILE *fp3,FILE *fp4){
    int funct=line&0x3f;
    rs=line>>21&0x1f;
    rt=line>>16&0x1f;
    rd=line>>11&0x1f;
    shamt=line>>6&0x1f;
    int rs_t=reg[rs],rt_t=reg[rt],err1=0;
    if(rd==0x00&&funct!=0x08){
        if(funct==0x00&&rt==0x00&&shamt==0) err1=0;
        else{
            err1=1;
            Error(1,fp4,fp3);
        }
    }
    if(funct==0x20){//add
        if(err1==0) reg[rd]=reg[rs]+reg[rt];
        if((rs_t>>31&0x1)==(rt_t>>31&&0x1)){
            if(((rs_t+rt_t)>>31&0x1)!=(rt_t>>31&&0x1)){
                Error(2,fp4,fp3);
            }
        }
    }
    else if(funct==0x22){//sub
        if(err1==0) reg[rd]=reg[rs]-reg[rt];
        //printf("%d %d %d\n",)
        /*if((rs_t>>31&0x1)!=(rt_t>>31&&0x1)){
                printf("%08X %08X %08X\n",rs_t,rt_t,(rs_t-rt_t));
            if(((rs_t-rt_t)>>31&0x1)!=(rs_t>>31&&0x1)){
                Error(2,fp4,fp3);
            }
        }*/
        if((rs_t<0&&(-rt_t)<0&&rs_t+(-rt_t)>=0)||(rs_t>0&&(-rt_t)>0&&rs_t+(-rt_t)<=0)){
            Error(2,fp4,fp3);
        }

    }
    else if(funct==0x24){//and
        if(err1==0) reg[rd]=reg[rs]&reg[rt];
    }
    else if(funct==0x25){//or
        if(err1==0) reg[rd]=reg[rs]|reg[rt];
    }
    else if(funct==0x26){//xor
        if(err1==0) reg[rd]=reg[rs]^reg[rt];
    }
    else if(funct==0x27){//nor
        if(err1==0) reg[rd]=~(reg[rs]|reg[rt]);
    }
    else if(funct==0x28){//nand
        if(err1==0) reg[rd]=~(reg[rs]&reg[rt]);
    }
    else if(funct==0x2A){//slt
        if(err1==0) reg[rd]=(reg[rs]<reg[rt]);
    }
    else if(funct==0x00){//sll
        if(err1==0) reg[rd]=reg[rt]<<shamt;
    }
    else if(funct==0x02){//srl
        if(err1==0){
            unsigned int data=(unsigned int)reg[rt]>>shamt;
            reg[rd]=data;
        }
    }
    else if(funct==0x03){//sra
        if(err1==0) reg[rd]=reg[rt]>>shamt;
    }
    else if(funct==0x08){//jr
        pc=reg[rs]-4;
    }
    else{
       // Error();
    }
    pc+=4;
    print(fp3);
}
void Itype(FILE *fp3,FILE *fp4){
    int imm_tmp=(line<<16)&0xFFFF0000;
    immediate=imm_tmp>>16;
    rt=line>>16&0x1F;
    rs=line>>21&0x1F;
    int rs_t=reg[rs],imm_t=immediate,err1=0;
    //printf("%       d\n",immediate);
    if(rt==0x00&&opcode!=0x2B&&opcode!=0x29&&opcode!=0x28&&opcode!=0x04&&opcode!=0x05) {
        err1=1;
        Error(1,fp4,fp3);
    }

    if(opcode==0x08){//addi
        if(err1==0) reg[rt]=reg[rs]+immediate;
        if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
            if(((rs_t+imm_t)>>31&0x1)!=(imm_t>>15&&0x1)){
                Error(2,fp4,fp3);
            }
        }

    }
    else if(opcode==0x23){//lw
            //printf("          %d\n",(rs_t+imm_t)/4);
        if((reg[rs]+immediate)%4==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1020||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                if(err1==0) reg[rt]=memory[(reg[rs]+immediate)/4];
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1020||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }


    }
    else if(opcode==0x21){//lh
        //printf("%08X %08X\n",rs_t,imm_t);
        if((reg[rs]+immediate)%2==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;

                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                if((reg[rs]+immediate)%4!=0){
                    int data=memory[(reg[rs]+immediate)/4]<<16&0xFFFF0000;
                    if(err1==0) reg[rt]=data>>16;
                }
                else {
                    int data=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
                    if(err1==0) reg[rt]=data>>16;
                }

            }
        }
        else {
                //printf("%08X %08X\n",rs_t,imm_t);
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;

                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x25){//lhu

        if((reg[rs]+immediate)%2==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                if((reg[rs]+immediate)%4!=0){
                    unsigned int data=memory[(reg[rs]+immediate)/4]<<16&0xFFFF0000;
                    if(err1==0) reg[rt]=data>>16;
                }
                else {
                    unsigned int data=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
                    if(err1==0) reg[rt]=data>>16;
                }
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x20){//lb
        if((reg[rs]+immediate)%4==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=memory[(reg[rs]+immediate)/4]&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==3){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=memory[(reg[rs]+immediate)/4]<<24&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==2){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=memory[(reg[rs]+immediate)/4]<<16&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==1) {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=memory[(reg[rs]+immediate)/4]<<8&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x24){//lbu
        if((reg[rs]+immediate)%4==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                unsigned int data=memory[(reg[rs]+immediate)/4]&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==3){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                unsigned int data=memory[(reg[rs]+immediate)/4]<<24&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==2){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                unsigned int data=memory[(reg[rs]+immediate)/4]<<16&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==1) {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                unsigned int data=memory[(reg[rs]+immediate)/4]<<8&0xFF000000;
                if(err1==0) reg[rt]=data>>24;
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x2B){//sw
        if((reg[rs]+immediate)%4==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1020||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                memory[(reg[rs]+immediate)/4]=reg[rt];
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1020||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x29){//sh
        if((reg[rs]+immediate)%2==0){
            if((reg[rs]+immediate)%4==0){
                if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                    int data=(rs_t+imm_t)/4;
                    if((data>>31&0x01)!=(rs_t>>31&0x1)){
                        Error(2,fp4,fp3);
                    }
                }
                if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                    Error(3,fp4,fp3);
                }
                else{
                    int data=reg[rt]<<16&0xFFFF0000;
                    memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0x0000FFFF;
                    memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
                }
            }
            else {
                if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                    int data=(rs_t+imm_t)/4;
                    if((data>>31&0x01)!=(rs_t>>31&0x1)){
                        Error(2,fp4,fp3);
                    }
                }
                if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                    Error(3,fp4,fp3);
                }
                else{
                    int data=reg[rt]&0x0000FFFF;
                    memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
                    memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
                }
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1022||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x28){//sb
        //printf(" add %d\n",(reg[rs]+immediate)/4);
        if((reg[rs]+immediate)%4==0){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=reg[rt]<<24&0xFF000000;
                memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0x00FFFFFF;
                memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
            }
        }
        else if((reg[rs]+immediate)%4==3){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=reg[rt]&0x000000FF;
                memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFFFF00;
                memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
            }
        }
        else if((reg[rs]+immediate)%4==2){
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=reg[rt]<<8&0x0000FF00;
                memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFF00FF;
                memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
            }
        }
        else if((reg[rs]+immediate)%4==1) {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            else{
                int data=reg[rt]<<16&0x00FF0000;
                memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFF00FFFF;
                memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
            }
        }
        else {
            if((rs_t>>31&0x1)==(imm_t>>15&&0x1)){
                int data=(rs_t+imm_t)/4;
                if((data>>31&0x01)!=(rs_t>>31&0x1)){
                    Error(2,fp4,fp3);
                }
            }
            if((rs_t+imm_t)>1023||(rs_t+imm_t)<0){
                Error(3,fp4,fp3);
            }
            Error(4,fp4,fp3);
        }
    }
    else if(opcode==0x0F){//lui
        if(err1==0) reg[rt]=immediate<<16;
    }
    else if(opcode==0x0C){//andi
        if(err1==0) reg[rt]=reg[rs]&((unsigned int)immediate&0x0000FFFF);
    }
    else if(opcode==0x0D){//ori
        if(err1==0) reg[rt]=reg[rs]|((unsigned int)immediate&0x0000FFFF);
    }
    else if(opcode==0x0E){//nori
        if(err1==0) reg[rt]=~(reg[rs]|((unsigned int)immediate&0x0000FFFF));
    }
    else if(opcode==0x0A){//slti
        if(err1==0) reg[rt]=(reg[rs]<immediate);
    }
    else if(opcode==0x04){//beq
        if((pc>>31&0x1)==(immediate>>15&0x1)){
            int data=pc+4*immediate;
            if((pc>>31&0x1)!=(data>>31&0x1)){
                Error(2,fp4,fp3);
            }
        }
        if(reg[rs]==reg[rt]) pc+=4*immediate;
    }
    else if(opcode==0x05){//bne
        if((pc>>31&0x1)==(immediate>>15&0x1)){
            int data=pc+4*immediate;
            if((pc>>31&0x1)!=(data>>31&0x1)){
                Error(2,fp4,fp3);
            }
        }
        if(reg[rs]!=reg[rt]) pc+=4*immediate;
    }
    else{
       // Error();
    }
    pc+=4;
    print(fp3);
}
void Jtype(FILE *fp3){
    address=line&0x3FFFFFF;
    if(opcode==0x02){//j
        pc=((pc+4)&0xF0000000)|(4*address);
    }
    else if(opcode==0x03){//jal
        reg[31]=pc+4;
        pc=((pc+4)&0xF0000000)|(4*address);
    }
    else{
      //  Error();
    }
    print(fp3);
}
int main()
{
    FILE *fp1=fopen("iimage.bin","rb");
    FILE *fp2=fopen("dimage.bin","rb");
    FILE *fp3=fopen("snapshot.rpt","w+");
    FILE *fp4=fopen("error_dump.rpt","w+");
    unsigned int tmp[500000];
    //puts("START");
        memset(reg,0,sizeof(reg));
    //if(fp1==NULL) puts("E");

    for(int i=0;fread(&tmp[i],sizeof(int),1,fp1)==1;i++){
        endian_swap(tmp[i]);
    }
    pc_in=tmp[0];
    num=tmp[1];
    for(int i=0;i<num;i++){
        //printf("%x\n",pc_in);
        //printf("%x\n",num);
        instruction[pc_in/4+i]=tmp[i+2];
       // system("PAUSE");
    }
   // system("PAUSE");
    for(int i=0;fread(&tmp[i],sizeof(int),1,fp2)==1;i++){
        endian_swap(tmp[i]);

    }
    //system("PAUSE");
    reg[29]=tmp[0];
    int memnum=tmp[1];
    for(int i=0;i<memnum;i++){
        memory[i]=tmp[i+2];
    }

    //main

    cycle=0;
    pc=pc_in;
    print(fp3);
    n=0;
    while(flag==0){
        opcode=instruction[pc/4]>>26&0x3f;
        //printf("%08X\n",opcode);
        line=instruction[pc/4];
        switch(opcode){
            case 0x00:
                Rtype(fp3,fp4);break;
            case 0x02:
            case 0x03:
                Jtype(fp3);break;
            case 0x04:
            case 0x05:
            case 0x08:
            case 0x0A:
            case 0x0C:
            case 0x0D:
            case 0x0E:
            case 0x0F:
            case 0x20:
            case 0x21:
            case 0x23:
            case 0x24:
            case 0x25:
            case 0x28:
            case 0x29:
            case 0x2B:
                Itype(fp3,fp4);break;
            case 0x3F:
                Halt();break;
            default:
               /* Error()*/break;
        }
        //system("PAUSE");
        //printf("%d\n",num);
        //printf("n=%d\n",n);
    }
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
}
