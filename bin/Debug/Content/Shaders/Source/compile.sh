# Requires glslangValidator installed from the Vulkan SDK
for filename in *.vert *.frag *.comp; do
    if [ -f $filename ]; then
        glslangValidator -V "$filename" -o "../Compiled/$filename.spv"
    fi
done
