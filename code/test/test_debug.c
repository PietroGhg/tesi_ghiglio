int main(){
  int a = 1;
  a++;
  a = a + 1;
}

/**
0000000000001120 <main>:
    1120:	55                   	push   %rbp
    1121:	48 89 e5             	mov    %rsp,%rbp
    1124:	31 c0                	xor    %eax,%eax
    1126:	c7 45 fc 01 00 00 00 	movl   $0x1,-0x4(%rbp)
    112d:	8b 4d fc             	mov    -0x4(%rbp),%ecx
    1130:	83 c1 01             	add    $0x1,%ecx
    1133:	89 4d fc             	mov    %ecx,-0x4(%rbp)
    1136:	8b 4d fc             	mov    -0x4(%rbp),%ecx
    1139:	83 c1 01             	add    $0x1,%ecx
    113c:	89 4d fc             	mov    %ecx,-0x4(%rbp)
    113f:	5d                   	pop    %rbp
    1140:	c3                   	retq   
    1141:	66 2e 0f 1f 84 00 00 	nopw   %cs:0x0(%rax,%rax,1)
    1148:	00 00 00 
    114b:	0f 1f 44 00 00       	nopl   0x0(%rax,%rax,1)
*/

/**
Address            Line   Column File   ISA Discriminator Flags
------------------ ------ ------ ------ --- ------------- -------------
0x0000000000001120      1      0      1   0             0  is_stmt
0x0000000000001126      2      7      1   0             0  is_stmt prologue_end
0x000000000000112d      3      4      1   0             0  is_stmt
0x0000000000001136      4      7      1   0             0  is_stmt
0x0000000000001139      4      9      1   0             0 
0x000000000000113c      4      5      1   0             0 
0x000000000000113f      5      1      1   0             0  is_stmt
0x0000000000001141      5      1      1   0             0  is_stmt end_sequence
*/
