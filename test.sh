# Usage:
# sh test.sh
# sh test.sh --show-log
# sh test.sh --file <filename>
# sh test.sh --file <filename>

sh build.sh
sleep 1


SHOW_LOG=false
SPECIFIC_FILE=""

while [[ "$#" -gt 0 ]]; do
    case $1 in
        --show-log) SHOW_LOG=true ;;
        --file) SPECIFIC_FILE="$2"; shift ;;
        *) echo "Unknown parameter: $1" ;;
    esac
    shift
done

cd build/src || exit

if [[ -n "$SPECIFIC_FILE" ]]; then
    file="../../test/$SPECIFIC_FILE"
    if [[ -f "$file" ]]; then
        filename=$(basename "$file")
        echo "🚀 Compiling $filename..."
        output=$(./compiler "$(cat "$file")" 2>&1)
        exit_code=$?
       echo "$output"

        if [[ $exit_code -eq 0 ]]; then
            echo "✅ $filename: Compilation successful"
        elif echo "$output" | grep -q "Syntax error"; then
            echo "❌ $filename: Syntax error"
        elif echo "$output" | grep -q "semantic is correct"; then
            echo "✅ $filename: Compilation successful (code gen errors are ignored)"
        else
            echo "✅ $filename: Compilation successful (ignored semantic errors)"
        fi
    else
        echo "❌ File '$SPECIFIC_FILE' not found in the ../../test directory."
    fi
else
    for file in ../../test/*; do
      
        if [[ -f "$file" ]]; then
            filename=$(basename "$file")
            echo "🚀 Compiling $filename..."
            output=$(./compiler "$(cat "$file")" 2>&1)
            exit_code=$?

            if [[ $SHOW_LOG == true ]]; then
                echo "$output"
            fi

            if [[ $exit_code -eq 0 ]]; then
                echo "✅ $filename: Compilation successful"
            elif echo "$output" | grep -q "Syntax error"; then
                echo "❌ $filename: Syntax error"
            elif echo "$output" | grep -q "semantic is correct"; then
                echo "✅ $filename: Compilation successful (semantic is correct)"
            else
                echo "✅ $filename: Compilation successful (ignored semantic errors)"
            fi
            echo "--------------------------------------------"
        else
            echo "No .txt files found in the directory."
        fi
    done
fi