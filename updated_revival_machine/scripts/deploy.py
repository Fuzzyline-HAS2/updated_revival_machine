import os
import re
import sys
import shutil
import subprocess
import glob

# Windows CMD에서 이모지 출력을 위한 설정
sys.stdout.reconfigure(encoding='utf-8')

# ================= 설정 =================
# 경로 구조:
#   scripts/deploy.py  <- 현재 파일
#   updated_revival_machine/  <- ARDUINO_DIR (스케치 폴더)
#   updated_revival_machine/  <- BASE_DIR (저장소 루트, 한 단계 위)
SCRIPT_DIR   = os.path.dirname(os.path.abspath(__file__))
ARDUINO_DIR  = os.path.dirname(SCRIPT_DIR)
BASE_DIR     = os.path.dirname(ARDUINO_DIR)

CONFIG_FILE        = os.path.join(ARDUINO_DIR, "OTA_Config.h")
OUTPUT_FILENAME    = "update.bin"
SIGNATURE_FILENAME = "update.sig"
# =======================================

# 비밀키는 secrets.py에서 관리 (GitHub에 올라가지 않음)
try:
    sys.path.insert(0, SCRIPT_DIR)
    from secrets import HMAC_SECRET
except ImportError:
    print("오류: scripts/secrets.py 파일이 없습니다.")
    print("   secrets.py.example을 secrets.py로 복사한 뒤 비밀키를 설정하세요.")
    sys.exit(1)

def get_current_version():
    with open(CONFIG_FILE, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.search(r'#define CURRENT_FIRMWARE_VERSION (\d+)', content)
    if match:
        return int(match.group(1))
    return None

def increment_version(current_ver):
    new_ver = current_ver + 1
    with open(CONFIG_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    new_content = re.sub(
        r'#define CURRENT_FIRMWARE_VERSION \d+',
        f'#define CURRENT_FIRMWARE_VERSION {new_ver}',
        content
    )

    with open(CONFIG_FILE, "w", encoding="utf-8") as f:
        f.write(new_content)
    return new_ver

def find_newest_bin():
    search_patterns = [
        os.path.join(ARDUINO_DIR, "build", "**", "*.bin"),
        os.path.join(BASE_DIR, "build", "**", "*.bin"),
        os.path.join(BASE_DIR, "**", "*.bin"),
    ]

    candidates = []
    for pattern in search_patterns:
        candidates.extend(glob.glob(pattern, recursive=True))

    # update.bin(배포용) 및 불필요한 바이너리 제외
    candidates = [f for f in candidates if not f.endswith(OUTPUT_FILENAME)]
    candidates = [f for f in candidates if "merged"     not in f]
    candidates = [f for f in candidates if "bootloader" not in f]
    candidates = [f for f in candidates if "partitions" not in f]
    candidates = [f for f in candidates if "boot_app"   not in f]

    if not candidates:
        return None

    return max(candidates, key=os.path.getmtime)

def git_push(version):
    print("\n GitHub에 업로드 중...")
    try:
        version_file = os.path.join(BASE_DIR, "version.txt")
        with open(version_file, "w", encoding="utf-8") as f:
            f.write(str(version))
        print(f"version.txt를 {version}으로 업데이트했습니다.")

        subprocess.run(["git", "add", "."], check=True, cwd=BASE_DIR)
        subprocess.run(
            ["git", "commit", "-m", f"Firmware Update v{version}"],
            check=True, cwd=BASE_DIR
        )
        subprocess.run(["git", "push"], check=True, cwd=BASE_DIR)
        print("업로드 완료!")
    except subprocess.CalledProcessError as e:
        print(f"Git 오류 발생: {e}")
        print("Git이 설치되어 있고 저장소가 연결되어 있는지 확인해주세요.")

def main():
    print("OTA 배포 자동화를 시작합니다...")

    # 1. 버전 확인 및 증가
    cur_ver = get_current_version()
    if cur_ver is None:
        print(f"오류: {CONFIG_FILE}에서 버전을 찾을 수 없습니다.")
        return

    print(f"현재 버전: {cur_ver}")
    new_ver = increment_version(cur_ver)
    print(f"버전을 {new_ver}로 변경했습니다.")

    # 2. 컴파일 대기
    print("\n[행동 필요] 이제 아두이노 IDE에서 '컴파일(Verify, Ctrl+R)' 버튼을 눌러주세요.")
    print("   컴파일이 완료되면 엔터(Enter) 키를 눌러주세요...")
    input()

    # 3. 파일 찾기
    print("빌드된 파일을 찾는 중...")
    bin_file = find_newest_bin()
    if not bin_file:
        print("오류: .bin 파일을 찾을 수 없습니다.")
        print("   빌드가 제대로 되었는지, build 폴더가 있는지 확인해주세요.")
        return

    print(f"   찾음: {bin_file}")

    # 4. 파일 복사
    output_path = os.path.join(BASE_DIR, OUTPUT_FILENAME)
    try:
        shutil.copy2(bin_file, output_path)
        print(f"파일을 '{OUTPUT_FILENAME}'으로 복사했습니다.")
    except Exception as e:
        print(f"파일 복사 실패: {e}")
        return

    # 5. 펌웨어 서명
    if HMAC_SECRET == "CHANGE_THIS_TO_YOUR_SECRET":
        print("오류: scripts/secrets.py의 HMAC_SECRET을 설정해주세요.")
        print("   secrets.h의 hmac_secret과 동일한 값으로 변경하세요.")
        return

    sign_script  = os.path.join(SCRIPT_DIR, "sign_firmware.py")
    sig_output   = os.path.join(BASE_DIR, SIGNATURE_FILENAME)
    result = subprocess.run(
        [sys.executable, sign_script, output_path, HMAC_SECRET, sig_output],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print(f"서명 실패:\n{result.stderr}")
        return
    print(f"{result.stdout.strip()}")

    # 6. Git 푸시
    git_push(new_ver)

    print(f"\n모든 작업이 완료되었습니다! 버전 {new_ver}이(가) 곧 배포됩니다.")

if __name__ == "__main__":
    main()
