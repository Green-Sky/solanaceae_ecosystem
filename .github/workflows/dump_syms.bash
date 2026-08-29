# needs TAG_NAME and REPO_NAME
set -e

# TODO: figure out apks

mkdir -p ./sym ./sym-stores

for archive in $(find ./artifacts -type f); do
	name=$(basename "$archive")
	case "$name" in
		*.tar.gz|*.tgz|*.tar|*.zip) : ;;
		*) continue ;;
	esac
	base="${name%.tar.gz}"; base="${base%.tgz}"; base="${base%.tar}"; base="${base%.zip}"

	IFS='-' read -ra p <<< "$base"
	n=${#p[@]}
	arch="${p[n-1]}"
	os=""; osidx=-1
	for (( i=n-2; i>=0; i-- )); do
		case "${p[i]}" in
			Linux|Windows|macOS) os="${p[i]}"; osidx=$i; break ;;
		esac
	done
	if [ "$osidx" -lt 0 ]; then
		echo "!! could not parse OS from $name" >&2
		continue
	fi
	variant=""
	for (( i=osidx+1; i<n-1; i++ )); do
		if [ -z "$variant" ]; then variant="${p[i]}"; else variant="${variant}-${p[i]}"; fi
	done

	store="${os}-${variant}-${arch}"
	extract="./extract/$store"
	rm -rf "$extract"
	mkdir -p "$extract" ./sym/"$store"

	# unar annoyingly fails for tars with self folder
	case "$name" in
		*.tar.gz|*.tgz|*.tar)	tar -x -f "$archive" -C "$extract" ;;
		*.zip)					unar -D -o "$extract/" "$archive" ;;
	esac

	if [ "$os" = Windows ]; then
		for f in $(find "$extract" -type f -name '*.pdb'); do
			dump_syms -s ./sym/"$store" "$f" >/dev/null 2>&1 || true
		done
	else
		for f in $(find "$extract" -type f); do
			dump_syms -s ./sym/"$store" "$f" >/dev/null 2>&1 || true
		done
		fi
	done

# compress
for d in ./sym/*/; do
	store="${d#./sym/}"; store="${store%/}"
	tar -czvf "./sym-stores/${REPO_NAME}-${TAG_NAME}-${store}-symbol_store.tar.gz" -C ./sym "$store"
done
