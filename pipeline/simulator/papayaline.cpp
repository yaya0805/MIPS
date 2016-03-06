#include<stdio.h>
#include<string.h>
#include<stdlib.h>
int reg[50],pc,num,instruction[500000],opcode,line,cycle,memory[300000],pc_in,n,flag=0,pc_next;
int halt_num;
int IF_ID,ID_EXE,EXE_MEM,MEM_WB;
int ID_EXE_rs,ID_EXE_rt,ID_EXE_rd,ID_EXE_shamt,ID_EXE_funct,ID_EXE_imme,ID_EXE_add,ID_EXE_type;
int EXE_MEM_ALU,EXE_MEM_type,EXE_MEM_rs,EXE_MEM_rt,EXE_MEM_rd,EXE_MEM_imme,EXE_MEM_funct;
int MEM_WB_MDR,MEM_WB_rs,MEM_WB_rt,MEM_WB_rd,MEM_WB_type,MEM_WB_ALU,MEM_WB_funct;
int WB_FIN,WB_FIN_rs,WB_FIN_rt,WB_FIN_rd,WB_FIN_type,WB_FIN_ALU,WB_FIN_funct,WB_FIN_MDR;
int FOR_ID_rs,FOR_ID_rt,FOR_ID_rs_data,FOR_ID_rt_data,FOR_EXE_rs,FOR_EXE_rt,FOR_EXE_rs_data,FOR_EXE_rt_data;
int stall,flush,JAL,to_be_stall;
int ID_EXE_op,EXE_MEM_op,MEM_WB_op,IF_START,WB_FIN_op;
int IF_NOP,ID_NOP,EXE_NOP,MEM_NOP,WB_NOP,MEM_STORE_SHIFT,WB_STORE_SHIFT;
inline void endian_swap(unsigned int& x)
{
    x = (x>>24) |((x<<8) & 0x00FF0000) |((x>>8) & 0x0000FF00) |(x<<24);
    //printf("%08X\n",x);
}
void Halt()
{
    halt_num++;
}
int IS_NOP(int tmp)
{
    //printf("%08X\n",tmp);
    if(tmp==0x00000000||tmp==0x00200000||tmp==0x00400000||tmp==0x00600000
       ||tmp==0x00800000||tmp==0x00A00000||tmp==0x00C00000||tmp==0x00E00000
       ||tmp==0x01000000||tmp==0x01200000||tmp==0x01400000||tmp==0x01600000
       ||tmp==0x01800000||tmp==0x01A00000||tmp==0x01C00000||tmp==0x01E00000
       ||tmp==0x02000000||tmp==0x02200000||tmp==0x02400000||tmp==0x02600000
       ||tmp==0x02800000||tmp==0x02A00000||tmp==0x02C00000||tmp==0x02E00000
       ||tmp==0x03000000||tmp==0x03200000||tmp==0x03400000||tmp==0x03600000
       ||tmp==0x03800000||tmp==0x03A00000||tmp==0x03C00000||tmp==0x03E00000){
        return 1;
        //puts("A");
       }
    else return 0;
}
char* Opcode_name(int ins,int f)
{
    int op=ins>>26&0x3f;
    switch(op){
        case 0x00:{
            int fun=ins&0x3f;
            switch(fun){
                case 0x00:
                    if(ins==0||f==1) {return "NOP";break;}
                    else {return "SLL";break;}
                case 0x02:
                    return "SRL";break;
                case 0x03:
                    return "SRA";break;
                case 0x08:
                    return "JR";break;
                case 0x20:
                    return "ADD";break;
                case 0x22:
                    return "SUB";break;
                case 0x24:
                    return "AND";break;
                case 0x25:
                    return "OR";break;
                case 0x26:
                    return "XOR";break;
                case 0x27:
                    return "NOR";break;
                case 0x28:
                    return "NAND";break;
                case 0x2A:
                    return "SLT";break;
                default:
                    return "WTH";break;
            }
            break;
        }
        case 0x02:
            return "J";break;
        case 0x03:
            return "JAL";break;
        case 0x04:
            return "BEQ";break;
        case 0x05:
            return "BNE";break;
        case 0x08:
            return "ADDI";break;
        case 0x0A:
            return "SLTI";break;
        case 0x0C:
            return "ANDI";break;
        case 0x0D:
            return "ORI";break;
        case 0x0E:
            return "NORI";break;
        case 0x0F:
            return "LUI";break;
        case 0x20:
            return "LB";break;
        case 0x21:
            return "LH";break;
        case 0x23:
            return "LW";break;
        case 0x24:
            return "LBU";break;
        case 0x25:
            return "LHU";break;
        case 0x28:
            return "SB";break;
        case 0x29:
            return "SH";break;
        case 0x2B:
            return "SW";break;
        case 0x3F:
            return "HALT";break;
        default:
           return "WTF";break;
    }
}
void Error(int n,FILE *fp4)
{
    if(n==1){
        fprintf(fp4,"In cycle %d: Write $0 Error\n",cycle+1);
        //printf("%d %08X\n",cycle+1,MEM_WB);
        //system("PAUSE");
    }
    else if(n==2){
        fprintf(fp4,"In cycle %d: Number Overflow\n",cycle+1);
    }
    else if(n==3){
        fprintf(fp4,"In cycle %d: Address Overflow\n",cycle+1);
        flag=1;
    }
    else if(n==4){
        fprintf(fp4,"In cycle %d: Misalignment Error\n",cycle+1);
        flag=1;
    }
}
void PrintReg(FILE *fp3)
{
    if(flag==0){
        fprintf(fp3,"cycle %d\n",cycle);
        for(int i=0;i<32;i++){
            fprintf(fp3,"$%02d: 0x%08X\n",i,reg[i]);
        }
        fprintf(fp3,"PC: 0x%08X\n",pc);
        /** PRINT **/
       /* printf("cycle %d\n",cycle);
        for(int i=0;i<32;i++){
            printf("$%02d: 0x%08X\n",i,reg[i]);
        }
        printf("PC: 0x%08X\n",pc);*/
    }
}
void Print(FILE *fp3)
{
        /** IF **/
        fprintf(fp3,"IF: 0x%08X",IF_ID);
        /** FLUSH **/
        if(flush==1){
            fprintf(fp3," to_be_flushed");
            IF_ID=0;
            flush=0;
            //pc-=4;
        }
        /** STALL **/
        if(to_be_stall!=0){
            fprintf(fp3," to_be_stalled");
            stall=to_be_stall;
            to_be_stall--;
        }
        fprintf(fp3,"\n");

        /** ID **/
        fprintf(fp3,"ID: %s",Opcode_name(ID_EXE,ID_NOP));
        /** STALL **/
        if(stall!=0){
            fprintf(fp3," to_be_stalled");
        }
        /** FORWARDING **/
        if(FOR_ID_rs!=0){
            if(MEM_WB_type=='R') fprintf(fp3," fwd_EX-DM_rs_$%d",MEM_WB_rd);
            else if(MEM_WB_type=='I'||MEM_WB_type=='M') fprintf(fp3," fwd_EX-DM_rs_$%d",MEM_WB_rt);
            else if(MEM_WB_op==0x03) fprintf(fp3," fwd_EX-DM_rs_$%d",ID_EXE_rs);
        }
        if(FOR_ID_rt!=0){
            if(MEM_WB_type=='R') fprintf(fp3," fwd_EX-DM_rt_$%d",MEM_WB_rd);
            else if(MEM_WB_type=='I'||MEM_WB_type=='M') fprintf(fp3," fwd_EX-DM_rt_$%d",MEM_WB_rt);
            else if(MEM_WB_op==0x03) fprintf(fp3," fwd_EX-DM_rt_$%d",ID_EXE_rt);
        }
        fprintf(fp3,"\n");

        /** EXE **/
        fprintf(fp3,"EX: %s",Opcode_name(EXE_MEM,EXE_NOP));
        /** FORWARDING **/
        if(FOR_EXE_rs==1){
            if(MEM_WB_type=='R') fprintf(fp3," fwd_EX-DM_rs_$%d",MEM_WB_rd);
            else if(MEM_WB_type=='I'||MEM_WB_type=='M') fprintf(fp3," fwd_EX-DM_rs_$%d",MEM_WB_rt);
            else if(MEM_WB_op==0x03) fprintf(fp3," fwd_EX-DM_rs_$%d",EXE_MEM_rs);
           // printf(" %d %d %d\n",cycle,EXE_MEM_rs,ID_EXE_rs);
        }
        if(FOR_EXE_rs==2){
            if(WB_FIN_type=='R') fprintf(fp3," fwd_DM-WB_rs_$%d",WB_FIN_rd);
            else if(WB_FIN_type=='I'||WB_FIN_type=='M') fprintf(fp3," fwd_DM-WB_rs_$%d",WB_FIN_rt);
            else if(WB_FIN_op==0x03) fprintf(fp3," fwd_DM-WB_rs_$%d",EXE_MEM_rs);
           // printf("%d %d %d\n",cycle,EXE_MEM_rs,WB_FIN_rs);
        }
        if(FOR_EXE_rt==1){
            if(MEM_WB_type=='R') fprintf(fp3," fwd_EX-DM_rt_$%d",MEM_WB_rd);
            else if(MEM_WB_type=='I'||MEM_WB_type=='M') fprintf(fp3," fwd_EX-DM_rt_$%d",MEM_WB_rt);
            else if(MEM_WB_op==0x03) fprintf(fp3," fwd_EX-DM_rt_$%d",EXE_MEM_rt);
        }
        if(FOR_EXE_rt==2){
            if(WB_FIN_type=='R') fprintf(fp3," fwd_DM-WB_rt_$%d",WB_FIN_rd);
            else if(WB_FIN_type=='I'||WB_FIN_type=='M') fprintf(fp3," fwd_DM-WB_rt_$%d",WB_FIN_rt);
            else if(WB_FIN_op==0x03) fprintf(fp3," fwd_DM-WB_rt_$%d",EXE_MEM_rt);
        }
        fprintf(fp3,"\n");

        /** MEM **/
        fprintf(fp3,"DM: %s\n",Opcode_name(MEM_WB,MEM_NOP));

        /** WB **/
        fprintf(fp3,"WB: %s\n\n\n",Opcode_name(WB_FIN,WB_NOP));
        FOR_ID_rs=0;
        FOR_ID_rt=0;
        FOR_EXE_rs=0;
        FOR_EXE_rt=0;

        cycle++;
        /*printf("%d\n",cycle-1);
        printf("0x%08X\n",IF_ID);
        printf("0x%08X\n",ID_EXE);
        printf("0x%08X\n",EXE_MEM);
        printf("0x%08X\n",MEM_WB);
        printf("0x%08X\n",WB_FIN);
        system("PAUSE");*/


}
void ID_Rtype()
{
    if(stall==0){
        /** DECODE **/
        ID_EXE_funct=IF_ID&0x3f;
        ID_EXE_rs=IF_ID>>21&0x1f;
        ID_EXE_rt=IF_ID>>16&0x1f;
        ID_EXE_rd=IF_ID>>11&0x1f;
        ID_EXE_shamt=IF_ID>>6&0x1f;
        ID_EXE_type='R';
        ID_EXE=IF_ID;
    }

    /** STALL **/
    if(EXE_MEM_op==0x23||EXE_MEM_op==0x21||EXE_MEM_op==0x25||EXE_MEM_op==0x20||EXE_MEM_op==0x24){/** LOAD **/
        if((ID_EXE_rs==EXE_MEM_rt&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03)||ID_EXE_rt==EXE_MEM_rt){
            if(EXE_MEM_rt!=0) to_be_stall=1;
        }
    }
    if(ID_EXE_funct==0x08){/** JR **/
        /** STALL **/
        if(MEM_WB_op==0x23||MEM_WB_op==0x21||MEM_WB_op==0x25||MEM_WB_op==0x20||MEM_WB_op==0x24){/** LOAD **/
            if(ID_EXE_rs==MEM_WB_rt){
                if(MEM_WB_rt!=0) to_be_stall=1;
            }
        }
        if(EXE_NOP==0){/** NOT NOP **/
            if(EXE_MEM_type=='I'||EXE_MEM_type=='M'){
                if(ID_EXE_rs==EXE_MEM_rt||ID_EXE_rt==EXE_MEM_rt){
                     if(EXE_MEM_rt!=0&&EXE_MEM_op!=0x2B&&EXE_MEM_op!=0x28&&EXE_MEM_op!=0x29) to_be_stall=1;
                }
            }
            else if(EXE_MEM_type=='R'){
                if(ID_EXE_rs==EXE_MEM_rd||ID_EXE_rt==EXE_MEM_rd){
                    if(EXE_MEM_rd!=0) to_be_stall=1;
                }
            }
        }

        /** FORWARDING (NO STALL)**/
        if(to_be_stall==0){
            /** R-TYPE **/
            if(MEM_WB_type=='R'&&MEM_WB_funct!=0x08){
                if(MEM_WB_rd==ID_EXE_rs&&MEM_WB_rd!=0){/** MEM_WB RS **/
                    FOR_ID_rs=1;
                    FOR_ID_rs_data=MEM_WB_ALU;
                }
            }

            /** I-TYPE **/
            if(MEM_WB_type=='I'||MEM_WB_type=='M'){
                if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0){/** MEM_WB RS **/
                    FOR_ID_rs=1;
                    FOR_ID_rs_data=MEM_WB_ALU;
                }
            }
            /** JAL **/
            if(MEM_WB_op==0x03){
                if(ID_EXE_rs==31){
                    FOR_ID_rs=1;
                    FOR_ID_rs_data=JAL;
                }
            }

        }
        int pc_tmp=reg[ID_EXE_rs];
        if(FOR_ID_rs!=0) pc_tmp=FOR_ID_rs_data;
        if(to_be_stall==0){
            if(pc_tmp!=(pc-4)){/** FLUSH **/
                flush=1;/** FLUSH IF_ID AFTER PRINT **/
            }
            pc_next=pc_tmp;
        }
    }
    //printf("%d\n",to_be_stall);
}
void ID_Jtype()
{
    if(stall==0){
        /** DECODE **/
        ID_EXE_add=IF_ID&0x3FFFFFF;
        ID_EXE_type='J';
        ID_EXE=IF_ID;
    }

    if(ID_EXE_op==0x02){/** J **/
        int pc_tmp=((pc)&0xF0000000)|(4*ID_EXE_add);
        //printf(" %08X\n",pc);
        //printf(" %08X\n",pc_tmp);
        if(to_be_stall==0){
            if(pc_tmp!=(pc-4)){/** FLUSH **/
                flush=1;/** FLUSH IF_ID AFTER PRINT **/
            }
            pc_next=pc_tmp;
        }
    }
    else if(ID_EXE_op==0x03){/** JAL **/
        JAL=pc;/** ? **/
        int pc_tmp=((pc)&0xF0000000)|(4*ID_EXE_add);
        //printf("0x%08X\n",pc_tmp);
        //system("PAUSE");
        if(to_be_stall==0){
            if(pc_tmp!=(pc-4)){/** FLUSH **/
                flush=1;/** FLUSH IF_ID AFTER PRINT **/
            }
            pc_next=pc_tmp;
        }
    }
}
void ID_Itype()
{
    if(stall==0){
        /** DECODE **/
        int ID_EXE_immetmp=(IF_ID<<16)&0xFFFF0000;
        ID_EXE_imme=ID_EXE_immetmp>>16;
        ID_EXE_rt=IF_ID>>16&0x1F;
        ID_EXE_rs=IF_ID>>21&0x1F;
        ID_EXE_type='I';
        ID_EXE=IF_ID;
    }


    /** STALL **/
    if(EXE_MEM_op==0x23||EXE_MEM_op==0x21||EXE_MEM_op==0x25||EXE_MEM_op==0x20||EXE_MEM_op==0x24){/** LOAD **/
        if(ID_EXE_rs==EXE_MEM_rt&&EXE_MEM_rt!=0){
            to_be_stall=1;
        }
    }
}
void ID_Branch(FILE *fp4)
{
    if(stall==0){
        /** DECODE **/
        int ID_EXE_immtmp=(IF_ID<<16)&0xFFFF0000;
        ID_EXE_imme=ID_EXE_immtmp>>16;
        ID_EXE_rt=IF_ID>>16&0x1F;
        ID_EXE_rs=IF_ID>>21&0x1F;
        ID_EXE_type='B';
        ID_EXE=IF_ID;
    }

    //printf("%d %d %d\n",ID_EXE_rs,ID_EXE_rt,MEM_WB_rt);

    /** STALL **/
    if(MEM_WB_op==0x23||MEM_WB_op==0x21||MEM_WB_op==0x25||MEM_WB_op==0x20||MEM_WB_op==0x24){/** LOAD **/
        if(ID_EXE_rs==MEM_WB_rt||ID_EXE_rt==MEM_WB_rt){
            if(MEM_WB_rt!=0) to_be_stall=1;
        }
    }
    if(EXE_NOP==0){/** NOT NOP **/
        if(EXE_MEM_type=='I'||EXE_MEM_type=='M'){
            if(ID_EXE_rs==EXE_MEM_rt||ID_EXE_rt==EXE_MEM_rt){
                 if(EXE_MEM_rt!=0&&EXE_MEM_op!=0x2B&&EXE_MEM_op!=0x28&&EXE_MEM_op!=0x29) to_be_stall=1;
            }
        }
        else if(EXE_MEM_type=='R'){
            if(ID_EXE_rs==EXE_MEM_rd||ID_EXE_rt==EXE_MEM_rd){
                if(EXE_MEM_rd!=0) to_be_stall=1;
            }
        }
    }

    /** FORWARDING (NO STALL)**/
    if(to_be_stall==0){
        /** R-TYPE **/
        if(MEM_WB_type=='R'&&MEM_WB_funct!=0x08){
            if(MEM_WB_rd==ID_EXE_rs&&MEM_WB_rd!=0){/** MEM_WB RS **/
                FOR_ID_rs=1;
                FOR_ID_rs_data=MEM_WB_ALU;
            }
            if(MEM_WB_rd==ID_EXE_rt&&MEM_WB_rd!=0){/** MEM_WB RT **/
                FOR_ID_rt=1;
                FOR_ID_rt_data=MEM_WB_ALU;
            }
        }

        /** I-TYPE **/
        if(MEM_WB_type=='I'){
            if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0){/** MEM_WB RS **/
                FOR_ID_rs=1;
                FOR_ID_rs_data=MEM_WB_ALU;
            }
            if(MEM_WB_rt==ID_EXE_rt&&MEM_WB_rt!=0){/** MEM_WB RS **/
                FOR_ID_rt=1;
                FOR_ID_rt_data=MEM_WB_ALU;
            }
        }
        /** JAL **/
        if(MEM_WB_op==0x03){
            if(ID_EXE_rs==31){
                FOR_ID_rs=1;
                FOR_ID_rs_data=JAL;
            }
            if(ID_EXE_rt==31){
                FOR_ID_rt=1;
                FOR_ID_rt_data=JAL;
            }
        }
    }

    int tmp_rs=reg[ID_EXE_rs],tmp_rt=reg[ID_EXE_rt];
    if(FOR_ID_rs==1) tmp_rs=FOR_ID_rs_data;
    if(FOR_ID_rt==1) tmp_rt=FOR_ID_rt_data;
    if(FOR_ID_rs==2) tmp_rs=FOR_ID_rs_data;
    if(FOR_ID_rt==2) tmp_rt=FOR_ID_rt_data;
    //printf("%02X\n",ID_EXE_op);
    if(ID_EXE_op==0x04){/** BEQ **/
        if((pc>>31&0x1)==(ID_EXE_imme>>15&0x1)){
            int data=pc+4*ID_EXE_imme;
            if((pc>>31&0x1)!=(data>>31&0x1)){
                Error(2,fp4);
            }
        }
        if(tmp_rs==tmp_rt){
            int pc_tmp=pc+4*ID_EXE_imme;
            if(to_be_stall==0){
                if(pc_tmp!=(pc-4)){/** FLUSH **/
                    flush=1;/** FLUSH IF_ID AFTER PRINT **/
                }
                //printf("%d 0x%08X 0x%08X\n",cycle,pc_tmp,instruction[pc_tmp/4]);
               // system("PAUSE");
                pc_next=pc_tmp;
            }
        }
    }
    else if(ID_EXE_op==0x05){/** BNE **/
        if((pc>>31&0x1)==(ID_EXE_imme>>15&0x1)){
            int data=pc+4*ID_EXE_imme;
            if((pc>>31&0x1)!=(data>>31&0x1)){
                Error(2,fp4);
            }
        }
        if(tmp_rs!=tmp_rt){
            int pc_tmp=pc+4*ID_EXE_imme;
            if(to_be_stall==0){
                //printf(" %08X\n",pc);
        //printf(" %08X\n",pc_tmp);
                if(pc_tmp!=(pc-4)){/** FLUSH **/
                    flush=1;/** FLUSH IF_ID AFTER PRINT **/
                }
                //printf("0x%08X\n",pc_tmp);
                //system("PAUSE");
                pc_next=pc_tmp;
            }
        }
    }
}
void ID_MemAccess()
{
    if(stall==0){
        /** DECODE **/
        int ID_EXE_immtmp=(IF_ID<<16)&0xFFFF0000;
        ID_EXE_imme=ID_EXE_immtmp>>16;
        ID_EXE_rt=IF_ID>>16&0x1F;
        ID_EXE_rs=IF_ID>>21&0x1F;
        ID_EXE_type='M';
        ID_EXE=IF_ID;
    }

    //printf("%d 0x%02X %d %d\n",cycle,ID_EXE_op,ID_EXE_rt,EXE_MEM_rt);
    //system("PAUSE");
    /** STALL **/
    if(EXE_MEM_op==0x23||EXE_MEM_op==0x21||EXE_MEM_op==0x25||EXE_MEM_op==0x20||EXE_MEM_op==0x24){/** LOAD **/
        if(ID_EXE_rs==EXE_MEM_rt){
            if(EXE_MEM_rt!=0) to_be_stall=1;
        }
        if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
            if(ID_EXE_rt==EXE_MEM_rt){
                if(EXE_MEM_rt!=0) to_be_stall=1;
            }
        }
    }
}
void EXE_Rtype(FILE *fp4)
{
    /** FORWARDING **/
    int tmp_rs=reg[ID_EXE_rs],tmp_rt=reg[ID_EXE_rt];
    if(FOR_EXE_rs==1) tmp_rs=FOR_EXE_rs_data;
    if(FOR_EXE_rt==1) tmp_rt=FOR_EXE_rt_data;
    if(FOR_EXE_rs==2) tmp_rs=FOR_EXE_rs_data;
    if(FOR_EXE_rt==2) tmp_rt=FOR_EXE_rt_data;

    /** ALU **/

    if(ID_EXE_funct==0x20){/** ADD **/
        EXE_MEM_ALU=tmp_rs+tmp_rt;
        //printf(" %d %d %d\n",tmp_rs,tmp_rt,EXE_MEM_ALU);
        /** NUM OVERFLOW **/
        if((tmp_rs>>31&0x1)==(tmp_rt>>31&&0x1)){
            if(((tmp_rs+tmp_rt)>>31&0x1)!=(tmp_rt>>31&&0x1)){
                Error(2,fp4);
            }
        }
    }
    else if(ID_EXE_funct==0x22){/** SUB **/
        EXE_MEM_ALU=tmp_rs-tmp_rt;

        /** NUM OVERFLOW **/
        if((tmp_rs<0&&(-tmp_rt)<0&&tmp_rs+(-tmp_rt)>=0)||(tmp_rs>0&&(-tmp_rt)>0&&tmp_rs+(-tmp_rt)<=0)){
            Error(2,fp4);
        }
    }
    else if(ID_EXE_funct==0x24){/** AND **/
        EXE_MEM_ALU=tmp_rs&tmp_rt;
    }
    else if(ID_EXE_funct==0x25){/** OR **/
        EXE_MEM_ALU=tmp_rs|tmp_rt;
    }
    else if(ID_EXE_funct==0x26){/** XOR **/
        EXE_MEM_ALU=tmp_rs^tmp_rt;
    }
    else if(ID_EXE_funct==0x27){/** NOR **/
        EXE_MEM_ALU=~(tmp_rs|tmp_rt);
    }
    else if(ID_EXE_funct==0x28){/** NAND **/
        EXE_MEM_ALU=~(tmp_rs&tmp_rt);
    }
    else if(ID_EXE_funct==0x2A){/** SLT **/
        EXE_MEM_ALU=(tmp_rs<tmp_rt);
    }
    else if(ID_EXE_funct==0x00){/** SLL **/
        EXE_MEM_ALU=tmp_rt<<ID_EXE_shamt;
    }
    else if(ID_EXE_funct==0x02){/** SRL **/
        unsigned int data=(unsigned int)tmp_rt>>ID_EXE_shamt;
        EXE_MEM_ALU=data;
    }
    else if(ID_EXE_funct==0x03){/** SRA **/
        EXE_MEM_ALU=tmp_rt>>ID_EXE_shamt;
    }

    /********************************** NOT SURE **********************************/
    else if(ID_EXE_funct==0x08){/** JR **/

    }

    EXE_MEM_rs=ID_EXE_rs;
    EXE_MEM_rt=ID_EXE_rt;
    EXE_MEM_rd=ID_EXE_rd;
}
void EXE_Jtype(FILE *fp4)
{

}
void EXE_Itype(FILE *fp4)
{
    /** FORWARDING **/
    int tmp_rs=reg[ID_EXE_rs];
    if(FOR_EXE_rs==1) tmp_rs=FOR_EXE_rs_data;
    if(FOR_EXE_rs==2) tmp_rs=FOR_EXE_rs_data;

    /** ALU **/

    if(ID_EXE_op==0x08){/** ADDI **/
        EXE_MEM_ALU=tmp_rs+ID_EXE_imme;

        /** NUM OVERFLOW **/
        if((tmp_rs>>31&0x1)==(ID_EXE_imme>>15&&0x1)){
            if(((tmp_rs+ID_EXE_imme)>>31&0x1)!=(ID_EXE_imme>>15&&0x1)){
                Error(2,fp4);
            }
        }
    }
    else if(ID_EXE_op==0x0F){/** LUI **/
        EXE_MEM_ALU=ID_EXE_imme<<16;
    }
    else if(ID_EXE_op==0x0C){/** ANDI **/
        EXE_MEM_ALU=tmp_rs&((unsigned int)ID_EXE_imme&0x0000FFFF);
    }
    else if(ID_EXE_op==0x0D){/** ORI **/
        EXE_MEM_ALU=tmp_rs|((unsigned int)ID_EXE_imme&0x0000FFFF);
    }
    else if(ID_EXE_op==0x0E){/** NORI **/
        EXE_MEM_ALU=~(tmp_rs|((unsigned int)ID_EXE_imme&0x0000FFFF));
    }
    else if(ID_EXE_op==0x0A){/** SLTI **/
        EXE_MEM_ALU=(tmp_rs<ID_EXE_imme);
    }

    EXE_MEM_rs=ID_EXE_rs;
    EXE_MEM_rt=ID_EXE_rt;
    EXE_MEM_imme=ID_EXE_imme;
}
void EXE_Branch(FILE *fp4)
{
    /********************************** NOT SURE **********************************/



}
void EXE_MemAccess(FILE *fp4)
{
    /** FORWARDING **/
    int tmp_rs=reg[ID_EXE_rs];
    int tmp_rt=reg[ID_EXE_rt];
    if(FOR_EXE_rs!=0) tmp_rs=FOR_EXE_rs_data;
    if(FOR_EXE_rt!=0) tmp_rt=FOR_EXE_rt_data;

    EXE_MEM_ALU=tmp_rs+ID_EXE_imme;

    /** NUM OVERFLOW **/
    if((tmp_rs>>31&0x1)==(ID_EXE_imme>>15&&0x1)){
        int data=(tmp_rs+ID_EXE_imme)/4;
        if((data>>31&0x01)!=(tmp_rs>>31&0x1)){
            Error(2,fp4);
        }
    }
    EXE_MEM_rs=ID_EXE_rs;
    EXE_MEM_rt=ID_EXE_rt;
    EXE_MEM_imme=ID_EXE_imme;
    if(ID_EXE_op==0x29){
        if((EXE_MEM_ALU)%4==0){
            MEM_STORE_SHIFT=tmp_rt<<16&0xFFFF0000;
        }
        else {
            MEM_STORE_SHIFT=tmp_rt&0x0000FFFF;
        }
    }
    else if(ID_EXE_op==0x28){
        if((EXE_MEM_ALU)%4==0){
            MEM_STORE_SHIFT=tmp_rt<<24&0xFF000000;
        }
        else if((EXE_MEM_ALU)%4==3){
            MEM_STORE_SHIFT=tmp_rt&0x000000FF;;
        }
        else if((EXE_MEM_ALU)%4==2){
            MEM_STORE_SHIFT=tmp_rt<<8&0x0000FF00;
        }
        else if((EXE_MEM_ALU)%4==1) {
            MEM_STORE_SHIFT=tmp_rt<<16&0x00FF0000;
        }
    }
}
void IF()
{
    /**  **/
    if(stall==0) IF_ID=instruction[pc/4];
    if(to_be_stall==0) pc=pc_next;
    //printf("%08X\n",pc);
    //if(stall==0) IF_START=instruction[pc/4];
    IF_NOP=0;
    IF_NOP=IS_NOP(IF_ID);
    //printf("%d\n",IF_NOP);
}
void ID(FILE *fp4)
{
    /** PC **/
    pc_next=pc+4;

     /** STALL **/
    //printf("%d\n",stall);
    if(stall!=0){
        EXE_MEM=0;
        EXE_MEM_ALU=0;
        EXE_MEM_type=0;
        EXE_MEM_rs=0;
        EXE_MEM_rt=0;
        EXE_MEM_rd=0;
        EXE_MEM_imme=0;
        EXE_MEM_funct=0;
        EXE_MEM_op=0;
       //printf("%d\n",stall);
        //return;
    }

    /** DECODE **/
    opcode=IF_ID>>26&0x3f;
    if(stall!=0) opcode=ID_EXE>>26&0x3f;
    ID_EXE_op=opcode;
    switch(opcode){
        case 0x00:
            ID_Rtype();break;
        case 0x02:
        case 0x03:
            ID_Jtype();break;
        case 0x04:
        case 0x05:
            ID_Branch(fp4);break;
        case 0x08:
        case 0x0A:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            ID_Itype();break;
        case 0x20:
        case 0x21:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x28:
        case 0x29:
        case 0x2B:
            ID_MemAccess();break;
        case 0x3F:
            Halt();break;
        default:
           break;
    }
    if(stall==0) {
        ID_EXE=IF_ID;
        ID_NOP=IS_NOP(ID_EXE);
    }
    if(stall!=0) {
        stall--;
    }

    //printf("0x%02X\n",ID_EXE_op);
}
void EXE(FILE *fp4)
{
    if(stall==0&&ID_EXE_op!=0x3F&&ID_EXE_op!=0x04&&ID_EXE_op!=0x05&&ID_EXE_funct!=0x08){/** IF STALL IS NOP **/
        /** FORWARDING **/

        /** R-TYPE **/
        if(MEM_WB_type=='R'&&MEM_WB_funct!=0x08){
            if(ID_EXE_type=='R'){
                if(MEM_WB_rd==ID_EXE_rs&&MEM_WB_rd!=0&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_ALU;
                }
                if(MEM_WB_rd==ID_EXE_rt&&MEM_WB_rd!=0){/** MEM_WB RT **/
                    FOR_EXE_rt=1;
                    FOR_EXE_rt_data=MEM_WB_ALU;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(MEM_WB_rd==ID_EXE_rs&&MEM_WB_rd!=0&&ID_EXE_op!=0x0F){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_ALU;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                    if(MEM_WB_rd==ID_EXE_rt&&MEM_WB_rd!=0){
                        FOR_EXE_rt=1;
                        FOR_EXE_rt_data=MEM_WB_ALU;
                    }
                }
            }
        }
        if(WB_FIN_type=='R'&&WB_FIN_funct!=0x08){
            if(ID_EXE_type=='R'){
                if(WB_FIN_rd==ID_EXE_rs&&WB_FIN_rd!=0&&FOR_EXE_rs!=1&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_ALU;
                }
                if(WB_FIN_rd==ID_EXE_rt&&WB_FIN_rd!=0&&FOR_EXE_rt!=1){/** WB_FIN RT **/
                    FOR_EXE_rt=2;
                    FOR_EXE_rt_data=WB_FIN_ALU;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(WB_FIN_rd==ID_EXE_rs&&WB_FIN_rd!=0&&FOR_EXE_rs!=1&&ID_EXE_op!=0x0F){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_ALU;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                    if(WB_FIN_rd==ID_EXE_rt&&WB_FIN_rd!=0&&FOR_EXE_rt!=1){
                        FOR_EXE_rt=2;
                        FOR_EXE_rt_data=WB_FIN_ALU;
                    }
                }
            }
        }


        /** I-TYPE **/
        if(MEM_WB_type=='I'){
            if(ID_EXE_type=='R'){
                if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_ALU;
                }
                if(MEM_WB_rt==ID_EXE_rt&&MEM_WB_rt!=0){/** MEM_WB RS **/
                    FOR_EXE_rt=1;
                    FOR_EXE_rt_data=MEM_WB_ALU;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                //printf("%d %d %d\n",cycle,MEM_WB_rt,ID_EXE_rs);
                //system("PAUSE");
                if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0&&ID_EXE_op!=0x0F){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_ALU;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                    if(MEM_WB_rt==ID_EXE_rt&&MEM_WB_rt!=0){
                        FOR_EXE_rt=1;
                        FOR_EXE_rt_data=MEM_WB_ALU;
                    }
                }
            }
        }
        if(WB_FIN_type=='I'){
            if(ID_EXE_type=='R'){
                if(WB_FIN_rt==ID_EXE_rs&&WB_FIN_rt!=0&&FOR_EXE_rs!=1&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_ALU;
                }
                if(WB_FIN_rt==ID_EXE_rt&&WB_FIN_rt!=0&&FOR_EXE_rt!=1){/** WB_FIN RS **/
                    FOR_EXE_rt=2;
                    FOR_EXE_rt_data=WB_FIN_ALU;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(WB_FIN_rt==ID_EXE_rs&&WB_FIN_rt!=0&&FOR_EXE_rs!=1&&ID_EXE_op!=0x0F){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_ALU;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                    if(WB_FIN_rt==ID_EXE_rt&&WB_FIN_rt!=0&&FOR_EXE_rt!=1){
                        FOR_EXE_rt=2;
                        FOR_EXE_rt_data=WB_FIN_ALU;
                    }
                }
            }
        }
        /** M-TYPE **/
        if(MEM_WB_type=='M'&&MEM_WB_op!=0x2B&&MEM_WB_op!=0x29&&MEM_WB_op!=0x28){
            if(ID_EXE_type=='R'){
                if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_MDR;
                }
                if(MEM_WB_rt==ID_EXE_rt&&MEM_WB_rt!=0){/** MEM_WB RS **/
                    FOR_EXE_rt=1;
                    FOR_EXE_rt_data=MEM_WB_MDR;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(MEM_WB_rt==ID_EXE_rs&&MEM_WB_rt!=0&&ID_EXE_op!=0x0F){/** MEM_WB RS **/
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=MEM_WB_MDR;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                    if(MEM_WB_rt==ID_EXE_rt&&MEM_WB_rt!=0){
                        FOR_EXE_rt=1;
                        FOR_EXE_rt_data=MEM_WB_MDR;
                    }
                }
            }
        }
        //printf("%d %02X\n",cycle,WB_FIN_op);
        //system("PAUSE");
        if(WB_FIN_type=='M'&&WB_FIN_op!=0x2B&&WB_FIN_op!=0x29&&WB_FIN_op!=0x28){
            if(ID_EXE_type=='R'){
                if(WB_FIN_rt==ID_EXE_rs&&WB_FIN_rt!=0&&FOR_EXE_rs!=1&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_MDR;
                }
                if(WB_FIN_rt==ID_EXE_rt&&WB_FIN_rt!=0&&FOR_EXE_rt!=1){/** WB_FIN RS **/
                    FOR_EXE_rt=2;
                    FOR_EXE_rt_data=WB_FIN_MDR;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                //printf("%d %d %d\n",cycle,WB_FIN_rt,ID_EXE_rs);
                //system("PAUSE");
                if(WB_FIN_rt==ID_EXE_rs&&WB_FIN_rt!=0&&FOR_EXE_rs!=1&&ID_EXE_op!=0x0F){/** WB_FIN RS **/
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=WB_FIN_MDR;
                }
                if(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28){
                         //printf("%d %d %d 0x%08X 0x%08X\n",cycle,WB_FIN_rt,ID_EXE_rt,ID_EXE,IF_ID);
                    if(WB_FIN_rt==ID_EXE_rt&&WB_FIN_rt!=0&&FOR_EXE_rt!=1){
                            //printf("%d\n",cycle);
                        FOR_EXE_rt=2;
                        FOR_EXE_rt_data=WB_FIN_MDR;
                    }
                }
            }
        }
        /** JAL **/
        if(MEM_WB_op==0x03){
            if(ID_EXE_type=='R'){
                if(ID_EXE_rs==31&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=JAL;
                }
                if(ID_EXE_rt==31&&ID_EXE_funct!=0x08){
                    FOR_EXE_rt=1;
                    FOR_EXE_rt_data=JAL;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(ID_EXE_rs==31&&ID_EXE_op!=0x0F){
                    FOR_EXE_rs=1;
                    FOR_EXE_rs_data=JAL;
                }
                if(ID_EXE_rt==31&&(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28)){
                    FOR_EXE_rt=1;
                    FOR_EXE_rt_data=JAL;
                }
            }
        }
        if(WB_FIN_op==0x03){
            if(ID_EXE_type=='R'){
                if(ID_EXE_rs==31&&ID_EXE_funct!=0x00&&ID_EXE_funct!=0x02&&ID_EXE_funct!=0x03){
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=JAL;
                }
                if(ID_EXE_rt==31&&ID_EXE_funct!=0x08){
                    FOR_EXE_rt=2;
                    FOR_EXE_rt_data=JAL;
                }
            }
            else if(ID_EXE_type=='I'||ID_EXE_type=='M'){
                if(ID_EXE_rs==31&&ID_EXE_op!=0x0F){
                    FOR_EXE_rs=2;
                    FOR_EXE_rs_data=JAL;
                }
                if(ID_EXE_rt==31&&(ID_EXE_op==0x2B||ID_EXE_op==0x29||ID_EXE_op==0x28)){
                    FOR_EXE_rt=2;
                    FOR_EXE_rt_data=JAL;
                }
            }
        }

        /** GO TO THE RIGHT FUCNT **/
        if(ID_EXE_type=='R') EXE_Rtype(fp4);
        else if(ID_EXE_type=='I') EXE_Itype(fp4);
        else if(ID_EXE_type=='J') EXE_Jtype(fp4);
        else if(ID_EXE_type=='B') EXE_Branch(fp4);
        else if(ID_EXE_type=='M') EXE_MemAccess(fp4);
    }
    if(stall==0){
        EXE_MEM=ID_EXE;
        EXE_MEM_type=ID_EXE_type;
        EXE_MEM_op=ID_EXE_op;
        EXE_NOP=IS_NOP(EXE_MEM);
        if(EXE_MEM_type=='R') EXE_MEM_funct=ID_EXE_funct;
    }
}
void MEM(FILE *fp4)
{
    /** ONLY M-TYPE & NEEDS **/
    if(EXE_MEM_type=='M'){
        if(EXE_MEM_op==0x23){/** LW **/
            if((EXE_MEM_ALU)%4==0){
                //printf("%08X\n",MEM_WB_MDR);
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1020||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }
                else{
                    MEM_WB_MDR=memory[(EXE_MEM_ALU)/4];
                }
            }
            else {
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1020||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }

                /** MISALIGNMENT ERROR **/
                Error(4,fp4);
            }
        }
        else if(EXE_MEM_op==0x21){/** LH **/
            if((EXE_MEM_ALU)%2==0){

                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }
                else{
                    if((EXE_MEM_ALU)%4!=0){
                        int data=memory[(EXE_MEM_ALU)/4]<<16&0xFFFF0000;
                        MEM_WB_MDR=data>>16;
                    }
                    else {
                        int data=memory[(EXE_MEM_ALU)/4]&0xFFFF0000;
                        MEM_WB_MDR=data>>16;
                    }
                }
            }
            else {
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }

                /** MISALIGNMENT ERROR **/
                Error(4,fp4);
            }
        }
        else if(EXE_MEM_op==0x25){/** LHU **/
            if((EXE_MEM_ALU)%2==0){

                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }
                else{
                    if((EXE_MEM_ALU)%4!=0){
                        unsigned int data=memory[(EXE_MEM_ALU)/4]<<16&0xFFFF0000;
                        MEM_WB_MDR=data>>16;
                    }
                    else {
                        unsigned int data=memory[(EXE_MEM_ALU)/4]&0xFFFF0000;
                        MEM_WB_MDR=data>>16;
                    }

                }
            }
            else {
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }

                /** MISALIGNMENT ERROR **/
                Error(4,fp4);
            }
        }
        else if(EXE_MEM_op==0x20){/** LB **/
            /** ADDRESS OVERFLOW **/
            if(EXE_MEM_ALU>1023||EXE_MEM_ALU<0){
                Error(3,fp4);
            }
            else {
                if((EXE_MEM_ALU)%4==0){
                    int data=memory[(EXE_MEM_ALU)/4]&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==3){
                    int data=memory[(EXE_MEM_ALU)/4]<<24&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==2){
                    int data=memory[(EXE_MEM_ALU)/4]<<16&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==1) {
                    int data=memory[(EXE_MEM_ALU)/4]<<8&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }

            }
        }
        else if(EXE_MEM_op==0x24){/** LBU **/
            /** ADDRESS OVERFLOW **/
            if(EXE_MEM_ALU>1023||EXE_MEM_ALU<0){
                Error(3,fp4);
            }
            else{
                if((EXE_MEM_ALU)%4==0){
                    unsigned int data=memory[(EXE_MEM_ALU)/4]&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==3){
                    unsigned int data=memory[(EXE_MEM_ALU)/4]<<24&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==2){
                    unsigned int data=memory[(EXE_MEM_ALU)/4]<<16&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
                else if((EXE_MEM_ALU)%4==1) {
                    unsigned int data=memory[(EXE_MEM_ALU)/4]<<8&0xFF000000;
                    MEM_WB_MDR=data>>24;
                }
            }
        }
        else if(EXE_MEM_op==0x2B){/** SW **/
            if((EXE_MEM_ALU)%4==0){
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1020||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }
                else{
                    memory[(EXE_MEM_ALU)/4]=reg[EXE_MEM_rt];
                }
            }
            else {
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1020||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }

                /** MISALIGNMENT ERROR **/
                Error(4,fp4);
            }
        }
        else if(EXE_MEM_op==0x29){/** SH **/
            if((EXE_MEM_ALU)%2==0){
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }
                else{
                    if((EXE_MEM_ALU)%4==0){
                        memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0x0000FFFF;
                        memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                    }
                    else {
                        memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0xFFFF0000;
                        memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                    }
                }
            }
            else {
                /** ADDRESS OVERFLOW **/
                if(EXE_MEM_ALU>1022||EXE_MEM_ALU<0){
                    Error(3,fp4);
                }

                /** MISALIGNMENT ERROR **/
                Error(4,fp4);
            }
        }
        else if(EXE_MEM_op==0x28){/** SB **/

            /** ADDRESS OVERFLOW **/
            if(EXE_MEM_ALU>1023||EXE_MEM_ALU<0){
                Error(3,fp4);
            }
            else{
                if((EXE_MEM_ALU)%4==0){
                    memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0x00FFFFFF;
                    memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                }
                else if((EXE_MEM_ALU)%4==3){
                    memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0xFFFFFF00;
                    memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                }
                else if((EXE_MEM_ALU)%4==2){
                    memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0xFFFF00FF;
                    memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                }
                else if((EXE_MEM_ALU)%4==1) {
                    memory[(EXE_MEM_ALU)/4]=memory[(EXE_MEM_ALU)/4]&0xFF00FFFF;
                    memory[(EXE_MEM_ALU)/4]=MEM_STORE_SHIFT|memory[(EXE_MEM_ALU)/4];
                }

            }
        }
    }
    else if(EXE_MEM_type=='I'||EXE_MEM_type=='R'){
        MEM_WB_ALU=EXE_MEM_ALU;
        //printf("      %d\n",MEM_WB_ALU);
    }

    MEM_WB=EXE_MEM;
    MEM_WB_rs=EXE_MEM_rs;
    MEM_WB_rt=EXE_MEM_rt;
    MEM_WB_rd=EXE_MEM_rd;
    MEM_WB_type=EXE_MEM_type;
    MEM_WB_op=EXE_MEM_op;
    MEM_NOP=IS_NOP(MEM_WB);
    if(MEM_WB_type=='R') MEM_WB_funct=EXE_MEM_funct;
}
void WB(FILE *fp4)
{
    //printf("%d %d %08X\n",MEM_WB_type-'A',MEM_WB_rt,MEM_WB_MDR);
    //printf("%d %d 0x%08X\n",cycle+1,MEM_WB_type-'A',MEM_WB);
    //system("PAUSE");
    if(MEM_WB_type=='M'){
        if(MEM_WB_op==0x23||MEM_WB_op==0x21||MEM_WB_op==0x25||MEM_WB_op==0x20||MEM_WB_op==0x24){
            /** WRITE $0 **/
            if(MEM_WB_rt==0&&IS_NOP(MEM_WB)==0&&MEM_WB_op!=0x3F) Error(1,fp4);
            else{
                reg[MEM_WB_rt]=MEM_WB_MDR;

            }
        }
    }
    else if(MEM_WB_type=='I'){
        /** WRITE $0 **/
        if(MEM_WB_rt==0&&IS_NOP(MEM_WB)==0&&MEM_WB_op!=0x3F) Error(1,fp4);
        else{
            reg[MEM_WB_rt]=MEM_WB_ALU;
        }
    }
    else if(MEM_WB_type=='R'&&MEM_WB_funct!=0x08){
        /** WRITE $0 **/
        if(MEM_WB_rd==0&&IS_NOP(MEM_WB)==0&&MEM_WB_op!=0x3F) Error(1,fp4);
        else{
            if(MEM_WB_funct!=0x08){/** JR **/
                reg[MEM_WB_rd]=MEM_WB_ALU;
                //printf("%d %d\n",MEM_WB_rd,MEM_WB_ALU);
                //system("PAUSE");
            }
        }
    }
    else if(MEM_WB_op==0x03){
        reg[31]=JAL;
    }
    WB_FIN=MEM_WB;
    WB_FIN_rs=MEM_WB_rs;
    WB_FIN_rt=MEM_WB_rt;
    WB_FIN_rd=MEM_WB_rd;
    WB_FIN_type=MEM_WB_type;
    WB_FIN_ALU=MEM_WB_ALU;
    WB_FIN_MDR=MEM_WB_MDR;
    WB_FIN_funct=MEM_WB_funct;
    WB_FIN_op=MEM_WB_op;
    WB_NOP=IS_NOP(WB_FIN);
}
int main()
{
    /** FILE OPEN **/
    FILE *fp1=fopen("iimage.bin","rb");
    FILE *fp2=fopen("dimage.bin","rb");
    FILE *fp3=fopen("snapshot.rpt","w+");
    FILE *fp4=fopen("error_dump.rpt","w+");

    /** INITIALIZE **/
    unsigned int tmp[500000];
    memset(reg,0,sizeof(reg));


    /** READIN **/
    fread(&tmp[0],sizeof(int),1,fp1);
    endian_swap(tmp[0]);
    pc_in=tmp[0];
    //printf("%08X\n",tmp[0]);
    fread(&tmp[1],sizeof(int),1,fp1);
    endian_swap(tmp[1]);
    num=tmp[1];
    //printf("%08X\n",tmp[1]);
    for(int i=2;fread(&tmp[i],sizeof(int),1,fp1)==1;i++){
        endian_swap(tmp[i]);
    }
    for(int i=0;i<num;i++){
        instruction[pc_in/4+i]=tmp[i+2];
        //printf("%d %08X\n",pc_in/4+i,instruction[pc_in/4+i]);
    }
    //system("PAUSE");



    /** DIMAGE **/
    for(int i=0;fread(&tmp[i],sizeof(int),1,fp2)==1;i++){
        endian_swap(tmp[i]);

    }
    reg[29]=tmp[0];
    int memnum=tmp[1];
    for(int i=0;i<memnum;i++){
        memory[i]=tmp[i+2];
    }
    //system("PAUSE");
    /** INITIALIZE **/
    cycle=0;
    pc=pc_in;
    halt_num=0;
    IF_ID=0;ID_EXE=0;EXE_MEM=0;MEM_WB=0;
    ID_EXE_rs=0;ID_EXE_rt=0;ID_EXE_rd=0;ID_EXE_shamt=0;ID_EXE_funct=0;ID_EXE_imme=0;ID_EXE_add=0;ID_EXE_type=0;
    EXE_MEM_ALU=0;EXE_MEM_type=0;EXE_MEM_rs=0;EXE_MEM_rt=0;EXE_MEM_rd=0;EXE_MEM_imme=0;EXE_MEM_funct=0;
    MEM_WB_MDR=0;MEM_WB_rs=0;MEM_WB_rt=0;MEM_WB_rd=0;MEM_WB_type=0;MEM_WB_ALU=0;MEM_WB_funct=0;WB_FIN=0;
    FOR_ID_rs=0;FOR_ID_rt=0;FOR_ID_rs_data=0;FOR_ID_rt_data=0;FOR_EXE_rs=0;FOR_EXE_rt=0;FOR_EXE_rs_data=0;FOR_EXE_rt_data=0;
    stall=0;flush=0;ID_EXE_op=0;EXE_MEM_op=0;MEM_WB_op=0;JAL=0;to_be_stall=0;WB_FIN_rs=0;WB_FIN_rt=0;WB_FIN_rd=0;
    WB_FIN_type=0;WB_FIN_ALU=0;WB_FIN_funct=0;WB_FIN_MDR=0;IF_NOP=1;ID_NOP=0;EXE_NOP=0;MEM_NOP=0;WB_NOP=0;
    MEM_STORE_SHIFT=0;WB_STORE_SHIFT=0;WB_FIN_op=0;
    IF_START=instruction[pc/4];
    //Print(fp3);

    /** MAIN **/
    while(halt_num!=4&&!flag){
        //printf(" %d %08X\n",ID_EXE_type-'A',ID_EXE_op);
        PrintReg(fp3);
        WB(fp4);
        //puts("A");
        MEM(fp4);
         //puts("B");
        EXE(fp4);
         //puts("C");
        ID(fp4);
         //puts("D");
        IF();
         //puts("E");
        Print(fp3);
    }
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    fclose(fp4);
}
