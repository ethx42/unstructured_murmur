for dir in */ ; do
    (cd "$dir" && find "$(pwd)" -type f ! -name '.*' ! -name '*.txt' > "$(basename "$(pwd)").txt")
done

