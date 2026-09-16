// Bubble Arena - 순수 C++ / Windows API (외부 라이브러리 없음)
// Visual Studio에서 "빈 C++ 프로젝트"를 만들고 이 파일을 추가한 뒤 F5를 누르세요.
#include <windows.h>
#include <vector>
#include <algorithm>
#include <string>

constexpr int COLS=15, ROWS=11, TILE=50, WIDTH=COLS*TILE, HEIGHT=ROWS*TILE;
enum Cell { EMPTY, WALL, CRATE };
struct Actor { int x=1,y=1,range=2; bool alive=true; float delay=0; };
struct Bubble { int x,y,range; float timer=2.05f; };
struct Flame { int x,y; float timer=.58f; };
Cell board[ROWS][COLS]; Actor player; std::vector<Bubble> bubbles; std::vector<Flame> flames;
bool ended=false; std::wstring message=L"상자를 모두 부숴보세요!";

bool Inside(int x,int y) { return x>=1&&y>=1&&x<COLS-1&&y<ROWS-1; }
bool HasBubble(int x,int y) { return std::any_of(bubbles.begin(),bubbles.end(),[=](const Bubble& b){return b.x==x&&b.y==y;}); }
bool HasFlame(int x,int y) { return std::any_of(flames.begin(),flames.end(),[=](const Flame& f){return f.x==x&&f.y==y;}); }
bool Blocked(int x,int y) { return !Inside(x,y)||board[y][x]!=EMPTY||HasBubble(x,y); }
void Reset() {
  bubbles.clear(); flames.clear(); ended=false; message=L"상자를 모두 부숴보세요!";
  for(int y=0;y<ROWS;y++) for(int x=0;x<COLS;x++) {
    board[y][x]=(x==0||y==0||x==COLS-1||y==ROWS-1||(x%2&&y%2))?WALL:EMPTY;
    if(board[y][x]==EMPTY && !(x<=2&&y<=2) && ((x+y*3)%3!=0)) board[y][x]=CRATE;
  }
  player={};
}
void AddFlame(int x,int y) { if(!HasFlame(x,y)) flames.push_back({x,y}); }
void Explode(Bubble b) {
  AddFlame(b.x,b.y); int dx[]={1,-1,0,0},dy[]={0,0,1,-1};
  for(int d=0;d<4;d++) for(int n=1;n<=b.range;n++) { int x=b.x+dx[d]*n,y=b.y+dy[d]*n;
    if(!Inside(x,y)||board[y][x]==WALL) break; AddFlame(x,y);
    if(board[y][x]==CRATE) { board[y][x]=EMPTY; break; }
  }
}
bool CratesRemain() { for(auto& row:board) for(Cell c:row) if(c==CRATE) return true; return false; }
void Update(float dt) {
  if(GetAsyncKeyState('R')&1) Reset(); if(ended) return;
  player.delay-=dt; int dx=0,dy=0;
  if(GetAsyncKeyState('W')||GetAsyncKeyState(VK_UP))dy=-1; else if(GetAsyncKeyState('S')||GetAsyncKeyState(VK_DOWN))dy=1;
  else if(GetAsyncKeyState('A')||GetAsyncKeyState(VK_LEFT))dx=-1; else if(GetAsyncKeyState('D')||GetAsyncKeyState(VK_RIGHT))dx=1;
  if(player.delay<=0 && (dx||dy) && !Blocked(player.x+dx,player.y+dy)) { player.x+=dx; player.y+=dy; player.delay=.12f; }
  if((GetAsyncKeyState(VK_SPACE)&1) && bubbles.empty()) bubbles.push_back({player.x,player.y,player.range});
  for(auto& b:bubbles)b.timer-=dt; std::vector<Bubble> boom;
  for(auto b:bubbles)if(b.timer<=0)boom.push_back(b);
  bubbles.erase(std::remove_if(bubbles.begin(),bubbles.end(),[](Bubble b){return b.timer<=0;}),bubbles.end());
  for(auto b:boom)Explode(b); for(auto& f:flames)f.timer-=dt;
  flames.erase(std::remove_if(flames.begin(),flames.end(),[](Flame f){return f.timer<=0;}),flames.end());
  if(HasFlame(player.x,player.y)){player.alive=false;ended=true;message=L"버블에 갇혔어요! R 키로 다시 도전";}
  else if(!CratesRemain()){ended=true;message=L"승리! 상자를 모두 부쉈어요. R 키로 다시 하기";}
}
void Fill(HDC dc,int x,int y,COLORREF color,int margin=0) { HBRUSH b=CreateSolidBrush(color); RECT r={x*TILE+margin,y*TILE+margin,(x+1)*TILE-margin,(y+1)*TILE-margin}; FillRect(dc,&r,b);DeleteObject(b); }
void Circle(HDC dc,int x,int y,int r,COLORREF color) { HBRUSH b=CreateSolidBrush(color);HBRUSH old=(HBRUSH)SelectObject(dc,b);Ellipse(dc,x-r,y-r,x+r,y+r);SelectObject(dc,old);DeleteObject(b); }
void Paint(HWND window) {
  PAINTSTRUCT ps; HDC dc=BeginPaint(window,&ps); SetBkMode(dc,TRANSPARENT);
  for(int y=0;y<ROWS;y++)for(int x=0;x<COLS;x++) { Fill(dc,x,y,(x+y)%2?RGB(99,189,220):RGB(112,200,231));
    if(board[y][x]==WALL){Fill(dc,x,y,RGB(23,78,113));Fill(dc,x,y,RGB(40,107,141),5);} if(board[y][x]==CRATE){Fill(dc,x,y,RGB(187,122,56));Fill(dc,x,y,RGB(229,166,85),6);}}
  for(auto f:flames){Circle(dc,f.x*TILE+25,f.y*TILE+25,21,RGB(223,250,255));Circle(dc,f.x*TILE+25,f.y*TILE+25,13,RGB(85,191,255));}
  for(auto b:bubbles){Circle(dc,b.x*TILE+25,b.y*TILE+26,17,RGB(22,127,202));Circle(dc,b.x*TILE+19,b.y*TILE+19,6,RGB(191,244,255));}
  if(player.alive){Circle(dc,player.x*TILE+25,player.y*TILE+25,17,RGB(255,207,82));RECT l={player.x*TILE+16,player.y*TILE+20,player.x*TILE+21,player.y*TILE+26},r={player.x*TILE+29,player.y*TILE+20,player.x*TILE+34,player.y*TILE+26};FillRect(dc,&l,(HBRUSH)GetStockObject(BLACK_BRUSH));FillRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));}
  SetTextColor(dc,RGB(255,255,255));TextOutW(dc,12,10,L"BUBBLE ARENA",13);SetTextColor(dc,RGB(65,40,0));TextOutW(dc,12,HEIGHT-30,message.c_str(),(int)message.size()); EndPaint(window,&ps);
}
LRESULT CALLBACK Proc(HWND w,UINT m,WPARAM p,LPARAM l) { if(m==WM_PAINT){Paint(w);return 0;} if(m==WM_DESTROY){PostQuitMessage(0);return 0;} return DefWindowProc(w,m,p,l); }
int WINAPI wWinMain(HINSTANCE h,HINSTANCE, PWSTR,int show) {
  WNDCLASS wc={};wc.hInstance=h;wc.lpszClassName=L"BubbleArena";wc.lpfnWndProc=Proc;wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);RegisterClass(&wc);
  HWND w=CreateWindow(wc.lpszClassName,L"Bubble Arena",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,CW_USEDEFAULT,CW_USEDEFAULT,WIDTH+16,HEIGHT+39,nullptr,nullptr,h,nullptr);ShowWindow(w,show);Reset();
  MSG msg={};DWORD last=GetTickCount();while(msg.message!=WM_QUIT){while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}DWORD now=GetTickCount();Update((now-last)/1000.f);last=now;InvalidateRect(w,nullptr,FALSE);Sleep(10);}return 0;
}
