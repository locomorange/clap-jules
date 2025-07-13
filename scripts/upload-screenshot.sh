#!/bin/bash
# upload-screenshot.sh - Upload screenshot to GitHub release and comment on PR
set -e

GITHUB_TOKEN="$1"
REPOSITORY="$2"
PR_NUMBER="$3"
RUN_ID="$4"
SHA="$5"
NEW_SCREENSHOT_HASH="$6"
SCREENSHOT_REASON="${7:-unknown}"

if [ -z "$GITHUB_TOKEN" ] || [ -z "$REPOSITORY" ] || [ -z "$PR_NUMBER" ]; then
    echo "Usage: $0 <github_token> <repository> <pr_number> <run_id> <sha> <new_screenshot_hash> [reason]"
    exit 1
fi

echo "=== Uploading screenshot to release ==="

# Check if screenshot exists
if [ ! -f "screenshots/clap-host-linux-screenshot.png" ]; then
    echo "No screenshot to upload"
    exit 0
fi

# Create unique filename with multiple identifiers to avoid conflicts
SCREENSHOT_FILENAME="clap-host-linux-pr${PR_NUMBER}-run${RUN_ID}-${SHA}.png"
cp screenshots/clap-host-linux-screenshot.png "$SCREENSHOT_FILENAME"

# Set up GitHub CLI authentication
export GITHUB_TOKEN="$GITHUB_TOKEN"

# Ensure shared screenshots release exists
echo "Checking for screenshots release..."
if ! gh release view screenshots --repo "$REPOSITORY" >/dev/null 2>&1; then
    gh release create screenshots --title "CLAP Host Screenshots" --notes "Shared screenshots from CLAP host integration tests" --repo "$REPOSITORY"
fi

# Upload screenshot to release
echo "Uploading screenshot to release..."
gh release upload screenshots "$SCREENSHOT_FILENAME" --clobber --repo "$REPOSITORY"

# Get the asset URL
ASSET_URL="https://github.com/${REPOSITORY}/releases/download/screenshots/${SCREENSHOT_FILENAME}"

# Comment on PR with screenshot
echo "Commenting on PR with screenshot..."

# Determine message based on reason
case "$SCREENSHOT_REASON" in
    "first_in_pr")
        REASON_MSG="このPRで初めてのスクリーンショットです。"
        ;;
    "changed_in_pr")
        REASON_MSG="このPR内で前回のスクリーンショットから変更されました。"
        ;;
    *)
        REASON_MSG="スクリーンショットが更新されました。"
        ;;
esac

COMMENT_BODY="## CLAP Host Screenshot (Linux)

${REASON_MSG}

📸 新しいスクリーンショット: [![screenshot](${ASSET_URL})](${ASSET_URL})

- 🔗 [全てのスクリーンショットを見る](https://github.com/${REPOSITORY}/releases/tag/screenshots)
- 📁 ファイル名: \`${SCREENSHOT_FILENAME}\`
- 🔄 更新理由: ${SCREENSHOT_REASON}

<!--screenshot-hash:${NEW_SCREENSHOT_HASH}-->"

gh pr comment "$PR_NUMBER" --body "$COMMENT_BODY" --repo "$REPOSITORY"

echo "✓ Screenshot upload and comment completed"
