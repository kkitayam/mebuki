#!/usr/bin/env python3
"""
Unity Test Framework 取得・配置スクリプト

使用方法:
  python scripts/update_unity.py          # 最新版取得
  python scripts/update_unity.py v2.5.2   # 指定バージョン取得
"""

import argparse
import json
import os
import shutil
import sys
import tempfile
import zipfile
from datetime import datetime
from pathlib import Path
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError


# 設定
GITHUB_REPO = "ThrowTheSwitch/Unity"
OUTPUT_DIR = Path("tests") / "unity"
REQUIRED_FILES = [
    "src/unity.c",
    "src/unity.h", 
    "src/unity_internals.h",
    "LICENSE.txt"
]


def get_latest_release():
    """GitHub APIから最新リリース情報を取得"""
    api_url = f"https://api.github.com/repos/{GITHUB_REPO}/releases/latest"
    
    try:
        req = Request(api_url)
        req.add_header('Accept', 'application/vnd.github.v3+json')
        
        with urlopen(req, timeout=10) as response:
            data = json.loads(response.read().decode())
            return data['tag_name']
    
    except (URLError, HTTPError, KeyError) as e:
        print(f"エラー: 最新リリース情報の取得に失敗しました: {e}", file=sys.stderr)
        return None


def download_unity(version, temp_dir):
    """指定バージョンのUnityをダウンロード"""
    # zipファイルURL
    zip_url = f"https://github.com/{GITHUB_REPO}/archive/refs/tags/{version}.zip"
    zip_path = temp_dir / f"unity_{version}.zip"
    
    print(f"ダウンロード中: {zip_url}")
    
    try:
        with urlopen(zip_url, timeout=30) as response:
            with open(zip_path, 'wb') as f:
                f.write(response.read())
        
        print(f"ダウンロード完了: {zip_path}")
        return zip_path
    
    except (URLError, HTTPError) as e:
        print(f"エラー: ダウンロードに失敗しました: {e}", file=sys.stderr)
        return None


def extract_files(zip_path, version, temp_dir):
    """ZIPから必要なファイルを抽出"""
    # 一時展開ディレクトリ
    extract_dir = temp_dir / "unity_extracted"
    
    # Unity-{version}形式のディレクトリ名（vプレフィックス除去）
    version_clean = version.lstrip('v')
    unity_dir = f"Unity-{version_clean}"
    
    print(f"展開中: {zip_path}")
    
    try:
        # 既存の展開ディレクトリを削除
        if extract_dir.exists():
            shutil.rmtree(extract_dir)
        
        # ZIP展開
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        
        # ファイル存在確認
        extracted_files = {}
        for file_path in REQUIRED_FILES:
            # LICENSE.txtの特別処理（ルートにある）
            if file_path == "LICENSE.txt":
                src = extract_dir / unity_dir / file_path
            else:
                src = extract_dir / unity_dir / file_path
            
            if not src.exists():
                print(f"警告: {file_path} が見つかりません", file=sys.stderr)
                continue
            
            extracted_files[file_path] = src
        
        if len(extracted_files) != len(REQUIRED_FILES):
            print("エラー: 必要なファイルが不足しています", file=sys.stderr)
            return None
        
        print(f"展開完了: {len(extracted_files)}ファイル")
        return extracted_files
    
    except zipfile.BadZipFile as e:
        print(f"エラー: ZIP展開に失敗しました: {e}", file=sys.stderr)
        return None


def copy_to_destination(extracted_files, version):
    """ファイルを最終配置場所にコピー"""
    # 出力ディレクトリ作成
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    
    print(f"コピー先: {OUTPUT_DIR}")
    
    # ファイルコピー
    for file_path, src in extracted_files.items():
        # 出力ファイル名（ディレクトリ構造を平坦化）
        filename = Path(file_path).name
        
        # LICENSE.txt -> LICENSE にリネーム
        if filename == "LICENSE.txt":
            filename = "LICENSE"
        
        dst = OUTPUT_DIR / filename
        
        shutil.copy2(src, dst)
        print(f"  コピー: {filename}")
    
    # VERSION.txt生成
    generate_version_file(version)
    
    print("配置完了")


def generate_version_file(version):
    """VERSION.txtを生成"""
    version_file = OUTPUT_DIR / "VERSION.txt"
    
    content = f"""Unity Test Framework {version}
https://github.com/{GITHUB_REPO}
Downloaded: {datetime.now().strftime('%Y-%m-%d')}
"""
    
    with open(version_file, 'w', encoding='utf-8') as f:
        f.write(content)
    
    print(f"  生成: VERSION.txt")


def cleanup(temp_dir):
    """一時ファイルのクリーンアップ"""
    if temp_dir.exists():
        shutil.rmtree(temp_dir)


def main():
    parser = argparse.ArgumentParser(
        description='Unity Test Frameworkを取得・配置します'
    )
    parser.add_argument(
        'version',
        nargs='?',
        default='latest',
        help='バージョン指定（例: v2.5.2）。省略時は最新版'
    )
    parser.add_argument(
        '--dry-run',
        action='store_true',
        help='実行内容を表示のみ（実際にはダウンロードしない）'
    )
    
    args = parser.parse_args()
    
    # バージョン決定
    if args.version == 'latest':
        print("最新バージョンを確認中...")
        version = get_latest_release()
        if not version:
            print("エラー: 最新バージョンの取得に失敗しました", file=sys.stderr)
            return 1
        print(f"最新バージョン: {version}")
    else:
        version = args.version
        print(f"指定バージョン: {version}")
    
    if args.dry_run:
        print(f"\n[DRY RUN] 以下の操作を実行します:")
        print(f"  1. {version} をダウンロード")
        print(f"  2. 必要ファイルを抽出")
        print(f"  3. {OUTPUT_DIR} に配置")
        return 0
    
    # 一時ディレクトリ作成（クロスプラットフォーム対応）
    temp_dir = Path(tempfile.mkdtemp(prefix='unity_'))
    
    try:
        # ダウンロード
        zip_path = download_unity(version, temp_dir)
        if not zip_path:
            return 1
        
        # 展開
        extracted_files = extract_files(zip_path, version, temp_dir)
        if not extracted_files:
            return 1
        
        # コピー
        copy_to_destination(extracted_files, version)
        
        print(f"\n成功: Unity {version} を {OUTPUT_DIR} に配置しました")
        return 0
    
    except Exception as e:
        print(f"エラー: 予期しないエラーが発生しました: {e}", file=sys.stderr)
        return 1
    
    finally:
        # クリーンアップ
        cleanup(temp_dir)


if __name__ == '__main__':
    sys.exit(main())
