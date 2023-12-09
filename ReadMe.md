# 2023 CSE2031 - libpng_project

## 개발 환경 설정

1. WSL - Ubuntu 22.04LTS 설치
1. libpng(제 3자 라이브러리) 설치

## git repository 받아오기

```
git clone [REPO_URL] [Local DIR]
```

## 코드 실행

```
g++ -Wall main.cpp -lpng -o main
./main
```

두 명령어 실행 이후 `out.png`를 확인하면 결과물을 볼 수 있습니다.