{*********************************************}
{***              Disassembler             ***}
{*** CPC to MSX transfered 1991 by the top ***}
{*** MSX to IBM transfered 1992 by the top ***}
{*********************************************}
program z80dis;
uses crt;
const
beftab : array[1..75] of string[4] =
 ('ADD ','ADC ','SUB ','SBC ','AND ','XOR ','OR  ','CP  ',
  'JR  ','NOP ','DJNZ','EX  ','RLCA','RLA ','DAA ','SCF ',
  'RRCA','RRA ','CPL ','CCF ','LD  ','INC ','DEC ','HALT',
  'RET ','POP ','JP  ','OUT ','EX  ','DI  ','CALL','PUSH',
  'RST ','EXX ','IN  ','EX  ','EI  ','LDI ','LDIR','INI ',
  'INIR','OUTI','OTIR','NEG ','RETN','RRD ','LDD ','LDDR',
  'CPD ','CPDR','IND ','INDR','OTDR','OUTD','RETI','RLD ',
  'BIT ','RES ','SET ','????','CPI ','CPIR','IM  ','----',
  'RLC ','RRC ','RL  ','RR  ','SLA ','SRA ','????','SRL ',
  'HL++','HL--','RDX ');
t1 : array[1..128] of byte =   {befehle 00..3f/c0..ff ohne ed/cb}
 (10,21,21,22,22,23,21,13,29,01,21,23,22,23,21,17,
  11,21,21,22,22,23,21,14,09,01,21,23,22,23,21,18,
  09,21,21,22,22,23,21,15,09,01,21,23,22,23,21,19,
  09,21,21,22,22,23,21,16,09,01,21,23,22,23,21,20,
  25,26,27,27,31,32,01,33,25,25,27,64,31,31,02,33,
  25,26,27,28,31,32,03,33,25,34,27,35,31,64,04,33,
  25,26,27,29,31,32,05,33,25,27,27,29,31,64,06,33,
  25,26,27,30,31,32,07,33,25,21,27,37,31,64,08,33);
t2 : array[1..96] of byte = {befehle 40..7f/eda0..edbf}
 (35,28,04,21,44,45,63,21,35,28,02,21,64,55,64,64,
  35,28,04,21,64,64,63,21,35,28,02,21,64,64,63,64,
  35,28,04,64,64,64,64,46,35,28,02,64,64,64,64,56,
  64,64,04,21,64,64,64,64,35,28,02,21,64,64,64,64,
  38,61,40,42,64,64,64,64,47,49,51,54,64,64,64,64,
  39,62,41,43,64,64,64,64,48,50,52,53,64,64,64,64);
t3 : array[1..128]of byte = {1.operand f.befehle 00..3f/c0..ff ohne ed/cb}
 ($ff,$30,$b0,$30,$10,$10,$10,$ff,$38,$34,$17,$30,$11,$11,$11,$ff,
  $41,$32,$b2,$32,$12,$12,$12,$ff,$41,$34,$17,$32,$13,$13,$13,$ff,
  $70,$34,$c4,$34,$14,$14,$14,$ff,$51,$34,$34,$34,$15,$15,$15,$ff,
  $72,$36,$c4,$36,$16,$16,$16,$ff,$11,$34,$17,$36,$17,$17,$17,$ff,
  $70,$30,$70,$44,$70,$30,$17,$20,$51,$ff,$51,$f1,$51,$44,$17,$21,
  $72,$32,$72,$c2,$72,$32,$42,$22,$53,$ff,$53,$17,$53,$f2,$17,$23,
  $74,$34,$74,$b6,$74,$34,$42,$24,$76,$16,$76,$32,$76,$f4,$42,$25,
  $58,$38,$58,$ff,$58,$38,$42,$26,$59,$36,$59,$ff,$59,$f8,$42,$27);
t4 : array[1..64] of byte = {1.operand f.befehle ed40..ed7f}
 ($10,$91,$34,$c4,$ff,$ff,$00,$1e,$11,$91,$34,$30,$ff,$ff,$ff,$1f,
  $12,$91,$34,$c4,$ff,$ff,$01,$17,$13,$91,$34,$32,$ff,$ff,$02,$17,
  $14,$91,$34,$c4,$ff,$ff,$76,$ff,$15,$91,$34,$34,$ff,$ff,$ff,$ff,
  $ff,$ff,$34,$c4,$ff,$ff,$ff,$ff,$17,$91,$34,$36,$ff,$ff,$ff,$ff);

t5 : array[1..128] of byte = {2.operand f.befehle 00..3f/c0..ff ohne ed/cb}
 ($ff,$44,$17,$ff,$ff,$ff,$42,$ff,$38,$30,$b0,$ff,$ff,$ff,$42,$ff,
  $ff,$44,$17,$ff,$ff,$ff,$42,$ff,$ff,$32,$b2,$ff,$ff,$ff,$42,$ff,
  $41,$44,$34,$ff,$ff,$ff,$42,$ff,$41,$34,$c4,$ff,$ff,$ff,$42,$ff,
  $41,$44,$17,$ff,$ff,$ff,$42,$ff,$41,$36,$c4,$ff,$ff,$ff,$42,$ff,
  $ff,$ff,$44,$ff,$44,$ff,$42,$ff,$ff,$ff,$44,$ff,$44,$ff,$42,$ff,
  $ff,$ff,$44,$17,$44,$ff,$ff,$ff,$ff,$ff,$44,$c2,$44,$ff,$42,$ff,
  $ff,$ff,$44,$34,$44,$ff,$ff,$ff,$ff,$ff,$44,$34,$44,$ff,$ff,$ff,
  $ff,$ff,$44,$ff,$44,$ff,$ff,$ff,$ff,$34,$44,$ff,$44,$ff,$ff,$ff);
t6 : array[1..64] of byte = {2.opereand f.befehle ed40..ed7f}
 ($91,$10,$30,$30,$ff,$ff,$ff,$17,$91,$11,$30,$c4,$ff,$ff,$ff,$17,
  $91,$12,$32,$32,$ff,$ff,$ff,$1e,$91,$13,$32,$c4,$ff,$ff,$ff,$1f,
  $91,$14,$34,$34,$ff,$ff,$ff,$ff,$91,$15,$34,$c4,$ff,$ff,$ff,$ff,
  $ff,$ff,$36,$36,$ff,$ff,$ff,$ff,$91,$17,$36,$c4,$ff,$ff,$ff,$ff);
var
 datef1           : file of byte;
 datef2           : text;
 datazone         : text;
 dataz            : boolean;
 datanf,datend    : word;
 nnr              : word;
 wert,zwert       : byte;
 adr,bis,fehadr   : longint;
 runsat           : longint;
 tlt,oprs,bef     : string;
 must,tust,tadr   : string;
 opr,buch,anz,indx: byte;
 qwert,off,nn,o2  : byte;
 b0,b1,b4,b5,b6,b7: byte;
 unknown          : boolean;
 zae              : longint;
 sl               : byte;
 modi             : char;
{-------------------------------------}
function xx(s1:string;n:byte):string;
begin
 xx:=s1+copy('                  ',1,n-length(string(s1)));
end;
{-------------------------------------}
function hex(n:longint;nn:byte):string;
var
 strn   : string;
 a,i    : byte;
begin
 strn:='';
 for i:=1 to nn do begin
  a:=(n and $f);
  if a>9 then inc(a,7);
  inc(a,$30);
  strn:=chr(a)+strn;
  n:=n div $10;
 end;
 hex:=strn;
end;
{-------------------------------------}
procedure sub1350;
begin
 if (wert<$20) or (wert>$7f) then tlt:='.' else tlt:=chr(wert);
end;
{-------------------------------------}
procedure sub1270;
begin
 read(datef1,wert);
 inc(adr);
 must:=must+hex(wert,2);
 sub1350;
 tust:=tust+tlt;
end;
{-------------------------------------}
procedure sub2050; {opereand aufbereiten}
begin
 if opr<>$ff then begin
  if opr=$17 then opr:=$18;   { A   umkodieren}
  if opr=$16 then opr:=$b4;   {(HL) umkodieren}
  if b7<>0 then oprs:=oprs+',';
  if (opr and $80)<>0 then oprs:=oprs+'(';
  buch:=opr and $f;
  anz:=1;
  case (opr and $70) of
   $00..$0f:oprs:=oprs+hex((opr and 7),1);   {bitnummer}
   $20..$2f:oprs:=oprs+hex((opr and 7)*8,2); {rst adr}
   $10..$1f,$30..$3f:begin                   {register}
             if (opr and $70)>$20 then inc(anz);
             if (opr and $7f)=$34 then if (b5<>0) and (qwert<>$eb) then begin
              buch:=10; b1:=1; if b6<>0 then buch:=12;
             end;
             oprs:=oprs+copy('BCDEHLSPAFIXIYIR',buch+1,anz);
             if (b0=0) and (opr=$b4) and (b5<>0) and (qwert<>$e9) then begin
              sub1270;
              off:=wert;
              b0:=1;    {HL & Indexreg. und nicht JP}
             end;
             if b0<>0 then begin oprs:=oprs+hex(off,2); b0:=0; end;
             if (opr and $80)<>0 then oprs:=oprs+')';
            end;
   $40..$4f,$60..$6f:begin                   {distanz}
             sub1270;
             if (opr and 1)=0 then begin {absolute zahl}
              nnr:=wert;
              anz:=2;
              if (opr and 2)=0 then begin
               sub1270; inc(nnr,wert*$100); anz:=4;
              end;
              oprs:=oprs+hex(nnr,anz);
              if (opr and $80)<>0 then oprs:=oprs+')';
             end else begin
              nnr:=adr+wert;
              if (wert and $80)<>0 then dec(nnr,256);
              oprs:=oprs+hex(nnr,4);
             end;
            end;
   $50..$5f:begin oprs:=oprs+copy('NZNCPOPEPM',buch+1,anz); end;
   $70..$7f:begin inc(anz); oprs:=oprs+copy('NZNCPOPEPM',buch+1,anz); end;
  end;
 end;
end;
{-------------------------------------}
procedure normbefehl;
begin
 case wert of
  $00..$3f:begin
            zwert:=wert+1;
            indx:=t1[zwert];
            opr:=t3[zwert];
            o2:=t5[zwert];
           end;
  $40..$7f:begin
            if wert=$76 then begin
             indx:=24;
             opr:=$ff;
             o2:=$ff;
            end else begin
             indx:=21;
             opr:=(wert and $38)div 8+$10;
             o2:=(wert and 7)+$10
            end;
           end;
  $80..$8f,
  $98..$9f:begin {operationen auf reg a (mit a angezeigt)}
            indx:=(wert and $38)div 8+1;
            opr:=$17;
            o2:=(wert and 7)+$10;
           end;
  $90..$97,
  $a0..$bf:begin {operationen auf reg a (ohne a angezeigen)}
            indx:=(wert and $38)div 8+1;
            opr:=(wert and 7)+$10;
            o2:=$ff;
           end;
  $c0..$ff:begin
            zwert:=wert-$80+1;
            indx:=t1[zwert];
            opr:=t3[zwert];
            o2:=t5[zwert];
           end;
 end;
end;
{-------------------------------------}
procedure edbefehl;
begin
 case wert of
  $00..$3f:unknown:=true;
  $40..$7f:begin
            zwert:=wert-$40+1;
            indx:=t2[zwert];
            opr:=t4[zwert];
            o2:=t6[zwert];
           end;
  $80..$9f:unknown:=true;
  $a0..$bf:begin
            indx:=t2[wert-$60+1];
            opr:=$ff;
            o2:=$ff;
           end;
  $c0..$ff:unknown:=true;
 end;
end;
{-------------------------------------}
procedure cbbefehl;
begin
 sub1270;
 if b5<>0 then begin
  off:=wert;
  b0:=1;
  sub1270;
 end;
 opr:=(wert and $38)div 8;
 o2:=(wert and 7)+$10;
 case wert of
  $00..$3f:begin        {Verschiebebefehle}
            indx:=opr+65;
            opr:=o2;
            o2:=$ff;
            if indx=71 then
             if (modi='G') and (wert=$37) then indx:=75 else indx:=64;
           end;
  $40..$7f:indx:=57;    {test bit}
  $80..$bf:indx:=58;    {reset bit}
  $c0..$ff:indx:=59;    {set bit}
 end;
end;
{-------------------------------------}
procedure disp;
begin
 if indx=64 then unknown:=true;
 bef:=beftab[indx];
 qwert:=wert;  {oder owert ??????}
 sub2050;
 b7:=1;
 opr:=o2;
 sub2050;
 if (b5<>0) and (b1=0) then unknown:=true;
 if unknown then begin
  adr:=fehadr;
  must:='';
  tust:='';
  seek(datef1,adr);
  sub1270;
  bef:='DB  ';
  oprs:=hex(wert,2);
 end;
 writeln(datef2,tadr,' ',xx(must,9),  {--xx(tust,5),--}  xx(bef,5),oprs);
end;
{-------------------------------------}
procedure disass;
label again;
begin
 oprs:='';
 must:='';
 tust:='';
 fehadr:=adr;
 unknown:=false;
 b0:=0; b1:=0; b4:=0; b5:=0; b6:=0; b7:=0;
 tadr:=hex(adr,5);
again:
 sub1270;
 if wert=$ed then
  if (b4 or b5)<>0 then unknown:=true else begin b4:=1; goto again; end;
 if (wert=$fd) then b6:=1;
 if (wert=$dd) or (wert=$fd) then
  if (b4 or b5)<>0 then unknown:=true else begin b5:=1; goto again; end;
 if b4<>0 then edbefehl else if wert=$cb then cbbefehl else normbefehl;
 disp;
end;
{-------------------------------------}
procedure data;
var
 s1 : string;
 c  : word;
begin
 tadr:=hex(adr,4);
 must:='';
 tust:='';
 sub1270;
 oprs:=hex(wert,2);
 sub1270;
 bef:='DEFW';
 oprs:=hex(wert,2)+oprs;
 writeln(datef2,tadr,' ',xx(must,9),  {--xx(tust,5),--}  xx(bef,5),oprs);
 if datend<adr then begin
  readln(datazone,s1);
  val(s1,datanf,c);
  readln(datazone,s1);
  val(s1,datend,c);
 end;
end;
{-------------------------------------}
procedure getparam;
var strn:string; code:integer; a:byte;
begin
{---}
 strn:=paramstr(1);
 while not ((length(strn)=1) and (upcase(strn[1]) in ['G','Z'])) do begin
  write('Z)80 mode or G)ameboy mode : ');
  readln(strn);
 end;
 modi:=upcase(strn[1]);
{---}
 {$I-}
 strn:=paramstr(2);
 if strn='' then strn:='*';
 assign(datef1,strn);
 reset(datef1);
 while ioresult<>0 do begin
  write('Sourcefile : ');
  readln(strn);
  assign(datef1,strn);
  reset(datef1);
 end;
{---}
 a:=pos('.',strn); if a=0 then a:=200;
 strn:=copy(strn,1,a-1)+'.DZO';
 assign(datazone,strn);
 {$I-}
 reset(datazone);
 {$I+}
 dataz:=(ioresult=0);
 if dataz then writeln('Datazonefile found!')
          else writeln('No datazonefile found.');
{---}
 strn:=paramstr(3);
 repeat
  if strn='' then begin
   write('Destinationfile [con] :');
   readln(strn);
   if strn='' then strn:='con';
  end;
  if strn='con' then sl:=23 else sl:=0;
  assign(datef2,strn);
  rewrite(datef2);
  if ioresult<>0 then strn:='';
 until strn<>'';
 {$I+}
{---}
 strn:=paramstr(4);
 repeat
  if strn='' then begin
   write('Startadr [00000] : ');
   readln(strn);
  end;
  if strn<>'' then val('$'+strn,adr,code) else begin adr:=0; code:=0; end;
  strn:='';
 until code=0;
 runsat:=adr;
 seek(datef1,runsat);
{---}
 strn:=paramstr(5);
 repeat
  bis:=filesize(datef1)-1;
  if strn='' then begin
   write('Endadr [',hex(bis,5),'] : ');
   readln(strn);
  end;
  if strn<>'' then val('$'+strn,bis,code) else code:=0;
  strn:='';
 until code=0;
 if bis>filesize(datef1)-1 then bis:=filesize(datef1)-1;
end;
{-------------------------------------}
procedure init;
begin
 writeln('Z80-Disassembler');
 writeln('Syntax: z80 <modi> <inputfile> <outputfile> <hexstart> <hexend>');
 writeln;
 getparam;
 if modi='G' then begin
  t1[$22+1]:=73; t3[$22+1]:=$16; t5[$22+1]:=$17;   {22 HL++ (HL),A  }
  t1[$2a+1]:=73; t3[$2a+1]:=$17; t5[$2a+1]:=$16;   {2A HL++ A,(HL)  }
  t1[$32+1]:=74; t3[$32+1]:=$16; t5[$32+1]:=$17;   {32 HL-- (HL),A  }
  t1[$3a+1]:=74; t3[$3a+1]:=$17; t5[$3a+1]:=$16;   {3A HL-- A,(HL) ?}
  t1[$53+1]:=71; t3[$53+1]:=$ff; t5[$53+1]:=$ff;   {D3 ? (ExOut)    }
  t1[$59+1]:=55; t3[$59+1]:=$ff; t5[$59+1]:=$ff;   {D9 RETI         }
  t1[$5b+1]:=71; t3[$5b+1]:=$ff; t5[$5b+1]:=$ff;   {DB ? (ExIn)     }
  t1[$60+1]:=28; t3[$60+1]:=$c2; t5[$60+1]:=$17;   {E0 OUT  (nn),A  }
  t1[$62+1]:=28; t3[$62+1]:=$91; t5[$62+1]:=$17;   {E2 OUT  (C),A  ?}
  t1[$6a+1]:=21; t3[$6a+1]:=$c4; t5[$6a+1]:=$17;   {EA LD   (nnnn),A}
  t1[$70+1]:=35; t3[$70+1]:=$17; t5[$70+1]:=$c2;   {F0 IN   A,(nn)  }
  t1[$7a+1]:=21; t3[$7a+1]:=$17; t5[$7a+1]:=$c4;   {FA LD   A,(nnnn)}
                                                 {CB37 RDX  A       }
 end;
end;
{-------------------------------------}
var
 s1 : string;
 c  : word;
begin
 init;
 zae:=0;
 if dataz then repeat
  readln(datazone,s1);
  val(s1,datanf,c);
  readln(datazone,s1);
  val(s1,datend,c);
 until datanf>=adr;
 while adr<=bis do begin
  if (not dataz) or ((adr<datanf) or (adr>datend)) then disass else data;
  tust:=tust+tlt;
  inc(zae);
  if zae=sl then begin zae:=0; if readkey=#$1b then bis:=0; end;
  if sl=0 then write(#13,tadr);
 end;
 writeln;
 close(datef1);
 close(datef2);
 if dataz then close(datazone);
end.
