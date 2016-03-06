#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
int reg[50],pc,num,instruction[1000000],opcode,line,cycle,memory[300000],pc_in,n,flag=0;
int rs,rt,rd,shamt,immediate,address;
int icache[1100][4],dcache[1100][4],itlb[1100][2],dtlb[1100][2],ipage[1100][3],dpage[1100][3];
int imem_size,ipage_size,icache_size,iblock_size,iset,dmem_size,dpage_size,dcache_size,dblock_size,dset,iTLB,iPTE,dTLB,dPTE;
int icache_H,icache_M,itlb_H,itlb_M,ipage_H,ipage_M,dcache_H,dcache_M,dtlb_H,dtlb_M,dpage_H,dpage_M;
int iTLB_num,iPTE_num,iPTE_PPN,icache_num,dTLB_num,dPTE_num,dPTE_PPN,dcache_num;
int MA,dirty,iPPN[1100],dPPN[1100];
inline void endian_swap(unsigned int& x)
{
    x = (x>>24) |((x<<8) & 0x00FF0000) |((x>>8) & 0x0000FF00) |(x<<24);
}
void print(FILE *fp3){
    if(flag==0){
        fprintf(fp3,"cycle %d\n",cycle);
         //printf("cycle %d\n",cycle);
         //printf("%02X\n",opcode);
        for(int i=0;i<32;i++){
            fprintf(fp3,"$%02d: 0x%08X\n",i,reg[i]);
            //printf("$%02d: 0x%08X\n",i,reg[i]);
        }
        fprintf(fp3,"PC: 0x%08X\n\n\n",pc);
        //printf("PC: 0x%08X\n\n\n",pc);
        cycle++;
    }
}
void Halt(FILE *fp4){
    flag=1;
    fprintf( fp4, "ICache :\n"); fprintf( fp4, "# hits: %u\n", icache_H ); fprintf( fp4, "# misses: %u\n\n", icache_M );
    fprintf( fp4, "DCache :\n"); fprintf( fp4, "# hits: %u\n", dcache_H ); fprintf( fp4, "# misses: %u\n\n", dcache_M );
    fprintf( fp4, "ITLB :\n"); fprintf( fp4, "# hits: %u\n", itlb_H ); fprintf( fp4, "# misses: %u\n\n", itlb_M );
    fprintf( fp4, "DTLB :\n"); fprintf( fp4, "# hits: %u\n", dtlb_H ); fprintf( fp4, "# misses: %u\n\n", dtlb_M );
    fprintf( fp4, "IPageTable :\n"); fprintf( fp4, "# hits: %u\n", ipage_H ); fprintf( fp4, "# misses: %u\n\n", ipage_M );
    fprintf( fp4, "DPageTable :\n"); fprintf( fp4, "# hits: %u\n", dpage_H ); fprintf( fp4, "# misses: %u\n\n", dpage_M );
}
void Dimage(FILE *fp4){
    /** TLB (FULLY_ASSOCIATIVE) **/
    int f=0;
    unsigned int l=MA/4;
    int tmp=l/dTLB_num;
   // printf("cycle %d %d %d\n",cycle,l,tmp);
    int MAX=1e9,nxt;
    for(int i=0;i<dTLB;i++){
        if(tmp==dtlb[i][0]&&dtlb[i][1]!=0){
            f=1;
            nxt=i;
            dtlb[i][1]=cycle;
            break;
        }
    }
    //printf("%d %d\n",cycle,nxt);
    //system("PAUSE");
    //printf("%d %08X\n",cycle,dtlb[nxt][0]);
    if(f==1) dtlb_H++;
    else{
        //printf("%d %d\n",cycle,nxt);
        dtlb_M++;
    }
    //dtlb[nxt][1]=cycle;

    /** PAGE TABLE **/
    unsigned int VPN;
    VPN=tmp;
    //printf("                %d\n",iPTE);
    if(f==0){

        //printf("%d %d %d\n",cycle,VPN,dpage[VPN][1]);
        if(dpage[VPN][0]==1){
            dpage_H++;
           // printf("    %d %d\n",cycle,dpage[VPN][1]);
            dPPN[dpage[VPN][1]]=cycle;
            //printf("H PPN:%d cycle:%d ",i,cycle);
        }
        else{
            MAX=1e9;
            for(int i=0;i<dPTE_num;i++){
              //  printf("M PPN:%d cycle:%d ",i,dPPN[i]);
                if((dPPN[i]<MAX)){
                    dPTE_PPN=i;
                    MAX=dPPN[i];
                }
            }
            //puts("");
            //printf("%d %d\n",cycle,dPTE_PPN);
            dPPN[dPTE_PPN]=cycle;
            int t_VPN=-1;
            for(int i=0;i<dPTE;i++){
                if(dpage[i][1]==dPTE_PPN&&dpage[i][0]!=0) {
                    dpage[i][0]=0;
                    t_VPN=i;
                   // printf("%d %d\n",cycle,t_VPN);
                }
            }
            //printf("%d\n",t_VPN);
            if(t_VPN!=-1){
                for(int i=0;i<dTLB;i++){
                    if(t_VPN==dtlb[i][0]&&dtlb[i][1]!=0){
                        dtlb[i][1]=0;
                        //puts("A");
                    }
                }
                int t_VA=dpage[t_VPN][2];
                int t_PA=(t_VA%dTLB_num+dpage[t_VPN][1]*dTLB_num);
                //int t_PA=dpage[t_VPN][1]*dTLB_num;
                int t_tmp7=t_PA%(dcache_num*dset);
                int t_st=dset*t_tmp7;
                int t_ed=t_st+dset;
                //printf("        %d %d %d\n",cycle,t_VPN,dpage[t_VPN][1]);
                for(int i=0;i<dcache_num;i++){
                    if(dcache[i][1]!=0&&dpage[t_VPN][1]==dcache[i][3]){
                       // puts("A");
                        dcache[i][1]=0;
                    }
                }
            }
            dpage[VPN][0]=1;
            dpage[VPN][1]=dPTE_PPN;
            dpage[VPN][2]=l;
            dpage_M++;
        }
                int MAX=1e9,nxt;
            for(int i=0;i<dTLB;i++){
                if(dtlb[i][1]<MAX){
                    nxt=i;
                    MAX=dtlb[i][1];
                }
            }
            dtlb[nxt][0]=VPN;
            dtlb[nxt][1]=cycle;
       // printf(" %d %d %d %d\n",cycle,VPN,dpage[VPN][1],dpage[VPN][2]);
    }

    /** CACHE **/
    int off=l%dTLB_num;
    int tmp5=dpage[VPN][1]*dTLB_num;
    int PA=off|tmp5;
    //printf("%d\n",dcache_num);
   // PA/=4;
    int tmp7=(PA/dblock_size)%(dcache_num/dset);
    int tmp8=(PA/dblock_size)/(dcache_num/dset);
    int st=dset*tmp7;
    int ed=st+dset;
    f=0;MAX=-1;nxt=-1;
    //printf("%d %d %d\n",cycle,tmp8,l);
        //printf("\n\n2:%d 3:%d \n",dcache[2][1],dcache[3][1]);
    //printf("c:%d VPN:%d PPN:%d VA:%d off:%d st:%d ed:%d PA:%d tmp7:%d ",cycle,VPN,dpage[VPN][1],l,off,st,ed,PA,tmp7);
    for(int i=st;i<ed;i++){
        //printf("i:%d valid bit:%d\n",i,dcache[i][1]);
        if(dcache[i][1]!=0) dcache[i][1]++;
        //printf("i:%d valid bit:%d\n",i,dcache[i][1]);
    }
    for(int i=st;i<ed;i++){
        if(l==dcache[i][0]&&dcache[i][1]!=0&&tmp8==dcache[i][2]){
            //printf("c:%d %d %d %d\n",cycle,VPN,dpage[VPN][1],l);
            f=1;
            nxt=i;
            dcache[i][1]=1;
            break;
        }
        else if((dcache[i][1]>MAX||dcache[i][1]==0)&&MAX!=0){
            nxt=i;
            MAX=dcache[i][1];
        }
    }
    if(f==1) dcache_H++;
    else{
        dcache[nxt][0]=l;
        dcache[nxt][2]=tmp8;
        dcache[nxt][3]=dpage[VPN][1];
        dcache_M++;
    }
    dcache[nxt][1]=1;
    //printf("i:%d f:%d\n",nxt,f);
}
void Iimage(FILE *fp4){
    /** TLB (FULLY_ASSOCIATIVE) **/
    int f=0;
    unsigned int l=pc/4;
    int tmp=l/iTLB_num;
   // printf("cycle %d %d %d\n",cycle,l,tmp);
    int MAX=1e9,nxt;
    for(int i=0;i<iTLB;i++){
        if(tmp==itlb[i][0]&&itlb[i][1]!=0){
            f=1;
            nxt=i;
            itlb[i][1]=cycle;
            break;
        }
    }
    //printf("%d %d\n",cycle,nxt);
    //system("PAUSE");
    //printf("%d %08X\n",cycle,dtlb[nxt][0]);
    if(f==1) itlb_H++;
    else{
        //printf("%d %d\n",cycle,nxt);
        itlb_M++;
    }
    //dtlb[nxt][1]=cycle;

    /** PAGE TABLE **/
    unsigned int VPN;
    VPN=tmp;
    //printf("                %d\n",iPTE);
    if(f==0){

        //printf("%d %d %d\n",cycle,VPN,dpage[VPN][1]);
        if(ipage[VPN][0]==1){
            ipage_H++;
           // printf("    %d %d\n",cycle,dpage[VPN][1]);
            iPPN[ipage[VPN][1]]=cycle;
            //printf("H PPN:%d cycle:%d ",i,cycle);
        }
        else{
            MAX=1e9;
            for(int i=0;i<iPTE_num;i++){
              //  printf("M PPN:%d cycle:%d ",i,dPPN[i]);
                if((iPPN[i]<MAX)){
                    iPTE_PPN=i;
                    MAX=iPPN[i];
                }
            }
            //puts("");
            //printf("%d %d\n",cycle,dPTE_PPN);
            iPPN[iPTE_PPN]=cycle;
            int t_VPN=-1;
            for(int i=0;i<iPTE;i++){
                if(ipage[i][1]==iPTE_PPN&&ipage[i][0]!=0) {
                    ipage[i][0]=0;
                    t_VPN=i;
                   // printf("%d %d\n",cycle,t_VPN);
                }
            }
            //printf("%d\n",t_VPN);
            if(t_VPN!=-1){
                for(int i=0;i<iTLB;i++){
                    if(t_VPN==itlb[i][0]&&itlb[i][1]!=0){
                        itlb[i][1]=0;
                        //puts("A");
                    }
                }
                int t_VA=ipage[t_VPN][2];
                int t_PA=(t_VA%iTLB_num+ipage[t_VPN][1]*iTLB_num);
                //int t_PA=dpage[t_VPN][1]*dTLB_num;
                int t_tmp7=t_PA%(icache_num*iset);
                int t_st=iset*t_tmp7;
                int t_ed=t_st+iset;
                //printf("        %d %d %d\n",cycle,t_VPN,dpage[t_VPN][1]);
                for(int i=0;i<icache_num;i++){
                    if(icache[i][1]!=0&&ipage[t_VPN][1]==icache[i][3]){
                       // puts("A");
                        icache[i][1]=0;
                    }
                }
            }
            ipage[VPN][0]=1;
            ipage[VPN][1]=iPTE_PPN;
            ipage[VPN][2]=l;
            ipage_M++;
        }
        int MAX=1e9,nxt;
        for(int i=0;i<iTLB;i++){
            if(itlb[i][1]<MAX){
                nxt=i;
                MAX=itlb[i][1];
            }
        }
        itlb[nxt][0]=VPN;
        itlb[nxt][1]=cycle;
       // printf(" %d %d %d %d\n",cycle,VPN,dpage[VPN][1],dpage[VPN][2]);
    }

    /** CACHE **/
    int off=l%iTLB_num;
    int tmp5=ipage[VPN][1]*iTLB_num;
    int PA=off|tmp5;
    //printf("%d\n",dcache_num);
   // PA/=4;
    int tmp7=(PA/iblock_size)%(icache_num/iset);
    int tmp8=(PA/iblock_size)/(icache_num/iset);
    int st=iset*tmp7;
    int ed=st+iset;
    f=0;MAX=-1;nxt=-1;
    //printf("%d %d %d\n",cycle,tmp8,l);
        //printf("\n\n2:%d 3:%d \n",dcache[2][1],dcache[3][1]);
    //printf("c:%d VPN:%d PPN:%d VA:%d off:%d st:%d ed:%d PA:%d tmp7:%d ",cycle,VPN,dpage[VPN][1],l,off,st,ed,PA,tmp7);
    for(int i=st;i<ed;i++){
        //printf("i:%d valid bit:%d\n",i,dcache[i][1]);
        if(icache[i][1]!=0) icache[i][1]++;
        //printf("i:%d valid bit:%d\n",i,dcache[i][1]);
    }
    for(int i=st;i<ed;i++){
        if(l==icache[i][0]&&icache[i][1]!=0&&tmp8==icache[i][2]){
            //printf("c:%d %d %d %d\n",cycle,VPN,dpage[VPN][1],l);
            f=1;
            nxt=i;
            icache[i][1]=1;
            break;
        }
        else if((icache[i][1]>MAX||icache[i][1]==0)&&MAX!=0){
            nxt=i;
            MAX=icache[i][1];
        }
    }
    if(f==1) icache_H++;
    else{
        icache[nxt][0]=l;
        icache[nxt][2]=tmp8;
        icache[nxt][3]=ipage[VPN][1];
        icache_M++;
    }
    icache[nxt][1]=1;
}
void Rtype(FILE *fp3){
    int funct=line&0x3f;
    rs=line>>21&0x1f;
    rt=line>>16&0x1f;
    rd=line>>11&0x1f;
    shamt=line>>6&0x1f;
    int rs_t=reg[rs],rt_t=reg[rt],err1=0;
    /** WRITE $0 **/
    if(rd==0x00&&funct!=0x08){
        if(funct==0x00&&rt==0x00&&shamt==0) err1=0;
        else err1=1;
    }

    if(funct==0x20){/** ADD **/
        if(err1==0) reg[rd]=reg[rs]+reg[rt];
    }
    else if(funct==0x22){/** SUB **/
        if(err1==0) reg[rd]=reg[rs]-reg[rt];
    }
    else if(funct==0x24){/** AND **/
        if(err1==0) reg[rd]=reg[rs]&reg[rt];
    }
    else if(funct==0x25){/** OR **/
        if(err1==0) reg[rd]=reg[rs]|reg[rt];
    }
    else if(funct==0x26){/** XOR **/
        if(err1==0) reg[rd]=reg[rs]^reg[rt];
    }
    else if(funct==0x27){/** NOR **/
        if(err1==0) reg[rd]=~(reg[rs]|reg[rt]);
    }
    else if(funct==0x28){/** NAND **/
        if(err1==0) reg[rd]=~(reg[rs]&reg[rt]);
    }
    else if(funct==0x2A){/** SLT **/
        if(err1==0) reg[rd]=(reg[rs]<reg[rt]);
    }
    else if(funct==0x00){/** SLL **/
        if(err1==0) reg[rd]=reg[rt]<<shamt;
    }
    else if(funct==0x02){/** SRL **/
        unsigned int data=(unsigned int)reg[rt]>>shamt;
        if(err1==0) reg[rd]=data;
    }
    else if(funct==0x03){/** SRA **/
        if(err1==0) reg[rd]=reg[rt]>>shamt;
    }
    else if(funct==0x08){/** JR **/
        pc=reg[rs]-4;
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
    /** WRITE $0 **/
    if(rt==0x00&&opcode!=0x2B&&opcode!=0x29&&opcode!=0x28&&opcode!=0x04&&opcode!=0x05) err1=1;
    else err1=0;

    if(opcode==0x08){/** ADDI **/
        if(err1==0) reg[rt]=reg[rs]+immediate;
    }
    else if(opcode==0x23){/** LW **/
        MA=(reg[rs]+immediate);
        if(err1==0){
            reg[rt]=memory[(reg[rs]+immediate)/4];
        }
        Dimage(fp4);
    }
    else if(opcode==0x21){/** LH **/
        MA=(reg[rs]+immediate);
        if((reg[rs]+immediate)%4!=0){
            int data=memory[(reg[rs]+immediate)/4]<<16&0xFFFF0000;
            if(err1==0) {
                reg[rt]=data>>16;
            }
        }
        else {
            int data=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
            if(err1==0){
                reg[rt]=data>>16;
            }
        }
        Dimage(fp4);
    }
    else if(opcode==0x25){/** LHU **/
        MA=(reg[rs]+immediate);
        if((reg[rs]+immediate)%4!=0){
            unsigned int data=memory[(reg[rs]+immediate)/4]<<16&0xFFFF0000;
            if(err1==0) {
                reg[rt]=data>>16;
            }
        }
        else {
            unsigned int data=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
            if(err1==0) {
                reg[rt]=data>>16;
            }
        }
        Dimage(fp4);
    }
    else if(opcode==0x20){/** LB **/
        MA=(reg[rs]+immediate);
        if((reg[rs]+immediate)%4==0){
            int data=memory[(reg[rs]+immediate)/4]&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==3){
            int data=memory[(reg[rs]+immediate)/4]<<24&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==2){
            int data=memory[(reg[rs]+immediate)/4]<<16&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==1) {
            int data=memory[(reg[rs]+immediate)/4]<<8&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        Dimage(fp4);
    }
    else if(opcode==0x24){/** LBU **/
        MA=(reg[rs]+immediate);
        if((reg[rs]+immediate)%4==0){
            unsigned int data=memory[(reg[rs]+immediate)/4]&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==3){
            unsigned int data=memory[(reg[rs]+immediate)/4]<<24&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==2){
            unsigned int data=memory[(reg[rs]+immediate)/4]<<16&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        else if((reg[rs]+immediate)%4==1) {
            unsigned int data=memory[(reg[rs]+immediate)/4]<<8&0xFF000000;
            if(err1==0){
                reg[rt]=data>>24;
            }
        }
        Dimage(fp4);
    }
    else if(opcode==0x2B){/** SW **/
        memory[(reg[rs]+immediate)/4]=reg[rt];
        MA=(reg[rs]+immediate);
        Dimage(fp4);
    }
    else if(opcode==0x29){/** SH **/
        if((reg[rs]+immediate)%4==0){
            int data=reg[rt]<<16&0xFFFF0000;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0x0000FFFF;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        else {
            int data=reg[rt]&0x0000FFFF;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFF0000;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        MA=(reg[rs]+immediate);
        Dimage(fp4);
    }
    else if(opcode==0x28){/** SB **/
        if((reg[rs]+immediate)%4==0){
            int data=reg[rt]<<24&0xFF000000;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0x00FFFFFF;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        else if((reg[rs]+immediate)%4==3){
            int data=reg[rt]&0x000000FF;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFFFF00;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        else if((reg[rs]+immediate)%4==2){
            int data=reg[rt]<<8&0x0000FF00;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFFFF00FF;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        else if((reg[rs]+immediate)%4==1) {
            int data=reg[rt]<<16&0x00FF0000;
            memory[(reg[rs]+immediate)/4]=memory[(reg[rs]+immediate)/4]&0xFF00FFFF;
            memory[(reg[rs]+immediate)/4]=data|memory[(reg[rs]+immediate)/4];
        }
        MA=(reg[rs]+immediate);
        Dimage(fp4);
    }
    else if(opcode==0x0F){/** LUI **/
        if(err1==0) reg[rt]=immediate<<16;
    }
    else if(opcode==0x0C){/** ANDI **/
        if(err1==0) reg[rt]=reg[rs]&((unsigned int)immediate&0x0000FFFF);
    }
    else if(opcode==0x0D){/** ORI **/
        if(err1==0) reg[rt]=reg[rs]|((unsigned int)immediate&0x0000FFFF);
    }
    else if(opcode==0x0E){/** NORI **/
        if(err1==0) reg[rt]=~(reg[rs]|((unsigned int)immediate&0x0000FFFF));
    }
    else if(opcode==0x0A){/** SLTI **/
        if(err1==0) reg[rt]=(reg[rs]<immediate);
    }
    else if(opcode==0x04){/** BEQ **/
        if(reg[rs]==reg[rt]) pc+=4*immediate;
    }
    else if(opcode==0x05){/** BNE **/
        if(reg[rs]!=reg[rt]) pc+=4*immediate;
    }
    pc+=4;
    print(fp3);
}
void Jtype(FILE *fp3){
    address=line&0x3FFFFFF;
    if(opcode==0x02){/** J **/
        pc=((pc+4)&0xF0000000)|(4*address);
    }
    else if(opcode==0x03){/** JAL **/
        reg[31]=pc+4;
        pc=((pc+4)&0xF0000000)|(4*address);
    }
    print(fp3);
}
int main(int argc,char *argv[])
{
    FILE *fp1=fopen("iimage.bin","rb");
    FILE *fp2=fopen("dimage.bin","rb");
    FILE *fp3=fopen("snapshot.rpt","w+");
    FILE *fp4=fopen("report.rpt","w+");
    unsigned int tmp[500000];
    memset(reg,0,sizeof(reg));
    imem_size=64,ipage_size=8,icache_size=16,iblock_size=4,iset=4;
    dmem_size=32,dpage_size=16,dcache_size=16,dblock_size=4,dset=1/* direct */;
    icache_H=0,icache_M=0,itlb_H=0,itlb_M=0,ipage_H=0,ipage_M=0;
    dcache_H=0,dcache_M=0,dtlb_H=0,dtlb_M=0,dpage_H=0,dpage_M=0;
    memset(icache,0,sizeof(icache));
    memset(dcache,0,sizeof(dcache));
    memset(itlb,0,sizeof(itlb));
    for(int i=0;i<1024;i++) itlb[i][0]=-1;
    memset(dtlb,0,sizeof(dtlb));
    for(int i=0;i<1024;i++) dtlb[i][0]=-1;
    memset(ipage,0,sizeof(ipage));
    memset(dpage,0,sizeof(dpage));
    memset(iPPN,0,sizeof(iPPN));
    memset(dPPN,0,sizeof(dPPN));
    for(int i=0;fread(&tmp[i],sizeof(int),1,fp1)==1;i++){
        endian_swap(tmp[i]);
    }
    pc_in=tmp[0];
    num=tmp[1];
    for(int i=0;i<(num<300?num:300);i++){
        instruction[pc_in/4+i]=tmp[i+2];
    }
    for(int i=0;fread(&tmp[i],sizeof(int),1,fp2)==1;i++){
        endian_swap(tmp[i]);
    }
    reg[29]=tmp[0];
    int memnum=tmp[1];
    for(int i=0;i<(memnum<300?memnum:300);i++){
        memory[i]=tmp[i+2];
    }

    /** MAIN **/
    if(argc!=1){
        sscanf(argv[1],"%d",&imem_size);
        sscanf(argv[2],"%d",&dmem_size);
        sscanf(argv[3],"%d",&ipage_size);
        sscanf(argv[4],"%d",&dpage_size);
        sscanf(argv[5],"%d",&icache_size);
        sscanf(argv[6],"%d",&iblock_size);
        sscanf(argv[7],"%d",&iset);
        sscanf(argv[8],"%d",&dcache_size);
        sscanf(argv[9],"%d",&dblock_size);
        sscanf(argv[10],"%d",&dset);
    }
    imem_size/=4,ipage_size/=4,icache_size/=4,iblock_size/=4;
    dmem_size/=4,dpage_size/=4,dcache_size/=4,dblock_size/=4;


    cycle=0;
    pc=pc_in;
    print(fp3);
    n=0;
    iPTE=256/ipage_size;iTLB=iPTE/4;
    dPTE=256/dpage_size;dTLB=dPTE/4;
    iTLB_num=ipage_size;
    iPTE_num=imem_size/ipage_size;
    icache_num=icache_size/iblock_size;
    iPTE_PPN=0;
    dTLB_num=dpage_size;
    dPTE_num=dmem_size/dpage_size;
    dcache_num=dcache_size/dblock_size;
    dPTE_PPN=0;
    dirty=0;
    while(flag==0){
        opcode=instruction[pc/4]>>26&0x3f;
        //printf("%08X\n",opcode);
        line=instruction[pc/4];
        Iimage(fp4);
        switch(opcode){
            case 0x00:
                Rtype(fp3);break;
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
                Halt(fp4);break;
            default:
               break;
        }

    }
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
}
