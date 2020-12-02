#include<iostream>
using namespace std;
int main(){
    string s="123456";
    int a=stoi(s);
    cout<<a+1<<endl;
    string s1=to_string(a);
    cout<<s1<<endl;
    char buff[10]="usama";
    string s2=buff;
    cout<<s2;

}