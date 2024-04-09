# ConsoleX

---

## 주요 기능

- "#00FF80" 같은 Hex 문자열을 이용한 터미널 폰트 / 배경 색상 설정 기능
- 즉시 키 입력값 반환 기능 ( Enter, Esc, Tab, F1 ~ F12 키 포함 )
- 터미널상의 커서 좌표 값 구하기 및 옮기기 기능

## 컴파일 방법

```shell
cd ${프로젝트 경로} # Makefile 있는 경로로 들어가기
make clean && make  # 기존 compile clean 후 새로 build 폴더에 컴파일
./build/runfile     # build 폴더에 있는 실행 파일(runfile) 실행
```