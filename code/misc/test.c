//here, with optimizations, line 4 and 10 are merged into a single one
//line location in debug info is line 0 (function declaration)
int f(int a, int b){
    if(a > b){
        a = a + b;
    }
    else{
        while(a <= b){
            a++;
        }
        a = a + b;
    }
    return a;
}

int main(){
  int a = f(10, 20);
  int b = f(11, 12);
}

/**
0000000000001120 <f>:
    1120:	55                   	push   %rbp
    1121:	48 89 e5             	mov    %rsp,%rbp
    1124:	89 7d fc             	mov    %edi,-0x4(%rbp)
    1127:	89 75 f8             	mov    %esi,-0x8(%rbp)
    112a:	8b 45 fc             	mov    -0x4(%rbp),%eax
    112d:	3b 45 f8             	cmp    -0x8(%rbp),%eax
    1130:	0f 8e 0e 00 00 00    	jle    1144 <f+0x24>
    1136:	8b 45 fc             	mov    -0x4(%rbp),%eax
    1139:	03 45 f8             	add    -0x8(%rbp),%eax
    113c:	89 45 fc             	mov    %eax,-0x4(%rbp)
    113f:	e9 28 00 00 00       	jmpq   116c <f+0x4c>
    1144:	e9 00 00 00 00       	jmpq   1149 <f+0x29>
    1149:	8b 45 fc             	mov    -0x4(%rbp),%eax
    114c:	3b 45 f8             	cmp    -0x8(%rbp),%eax
    114f:	0f 8f 0e 00 00 00    	jg     1163 <f+0x43>
    1155:	8b 45 fc             	mov    -0x4(%rbp),%eax
    1158:	83 c0 01             	add    $0x1,%eax
    115b:	89 45 fc             	mov    %eax,-0x4(%rbp)
    115e:	e9 e6 ff ff ff       	jmpq   1149 <f+0x29>
    1163:	8b 45 fc             	mov    -0x4(%rbp),%eax
    1166:	03 45 f8             	add    -0x8(%rbp),%eax
    1169:	89 45 fc             	mov    %eax,-0x4(%rbp)
    116c:	8b 45 fc             	mov    -0x4(%rbp),%eax
    116f:	5d                   	pop    %rbp
    1170:	c3                   	retq   
    1171:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    1178:	00 00 00 
    117b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
*/

/**
Address            Line   Column File   ISA Discriminator Flags
------------------ ------ ------ ------ --- ------------- -------------
0x0000000000001120      3      0      1   0             0  is_stmt
0x000000000000112a      4      8      1   0             0  is_stmt prologue_end
0x000000000000112d      4     10      1   0             0 
0x0000000000001130      4      8      1   0             0 
0x0000000000001136      5     13      1   0             0  is_stmt
0x0000000000001139      5     15      1   0             0 
0x000000000000113c      5     11      1   0             0 
0x000000000000113f      6      5      1   0             0  is_stmt
0x0000000000001144      8      9      1   0             0  is_stmt
0x0000000000001149      8     15      1   0             0 
0x000000000000114c      8     17      1   0             0 
0x000000000000114f      8      9      1   0             0 
0x0000000000001155      9     14      1   0             0  is_stmt
0x000000000000115e      8      9      1   0             0  is_stmt
0x0000000000001163     11     13      1   0             0  is_stmt
0x0000000000001166     11     15      1   0             0 
0x0000000000001169     11     11      1   0             0 
0x000000000000116c     13     12      1   0             0  is_stmt
0x000000000000116f     13      5      1   0             0 
0x0000000000001180     16      0      1   0             0  is_stmt
0x0000000000001188     17     11      1   0             0  is_stmt prologue_end
0x0000000000001197     17      7      1   0             0 
0x000000000000119a     18     11      1   0             0  is_stmt
0x00000000000011ab     18      7      1   0             0 
0x00000000000011ae     19      1      1   0             0  is_stmt
0x00000000000011b6     19      1      1   0             0  is_stmt end_sequence
*/
