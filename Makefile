# =========================================================================== #
#                                 Verbosité                                   #
# =========================================================================== #
ifeq ($(V),1)
  Q :=
else
  Q := @
endif

# +-------------------------------------------------------------------------+ #
# |                              Chemins système                            | #
# +-------------------------------------------------------------------------+ #
NDK         := $(HOME)/.android-ndk/android-ndk-r29
HOST_TAG    := $(shell uname -s | tr '[:upper:]' '[:lower:]')-$(shell uname -m)
TOOLCHAIN   := $(NDK)/toolchains/llvm/prebuilt/$(HOST_TAG)
SYSROOT     := $(TOOLCHAIN)/sysroot
SYSROOT_LIB := $(SYSROOT)/usr/lib
CLANG       := $(TOOLCHAIN)/bin/clang
ANDROID_JAR := toolz/android.jar
API         := 24
GLUE        := $(NDK)/sources/android/native_app_glue

# +-------------------------------------------------------------------------+ #
# |                                  Flags                                  | #
# +-------------------------------------------------------------------------+ #
CFLAGS_USER := -O3 -fPIC -Wall -Wextra -Werror
CFLAGS_GLUE := -O3 -fPIC
INCLUDES    := -I$(GLUE) -Isrc
LDFLAGS     := -shared -fPIC -u ANativeActivity_onCreate -landroid -llog -lm

# +-------------------------------------------------------------------------+ #
# |                               ABI cibles                                | #
# +-------------------------------------------------------------------------+ #
ARCHS := arm64-v8a x86_64

TRIPLE_arm64-v8a := aarch64-linux-android$(API)
TRIPLE_x86_64    := x86_64-linux-android$(API)

PREFIX_arm64-v8a := aarch64-linux-android
PREFIX_x86_64    := x86_64-linux-android

ARCHS := $(foreach a,$(ARCHS),\
    $(if $(wildcard $(SYSROOT_LIB)/$(PREFIX_$(a))/$(API)),$(a)))

# +-------------------------------------------------------------------------+ #
# |                                  Sources                                | #
# +-------------------------------------------------------------------------+ #
SRCS         := $(wildcard src/*/*.c)
APKS         := $(foreach a,$(ARCHS),build/app-$a.apk)
COPY_TARGETS := $(addprefix copy-,$(ARCHS))

# +-------------------------------------------------------------------------+ #
# |                                  Cibles                                 | #
# +-------------------------------------------------------------------------+ #
.PHONY: all clean $(COPY_TARGETS)

all: $(APKS) $(COPY_TARGETS)
	@printf "== Terminé ==\n"
	@ls -lh build/app-*.apk

clean:
	$(Q)rm -rf build lib

build build/obj:
	$(Q)mkdir -p $@

debug.keystore:
	@printf "== Génération de la clé de debug ==\n"
	$(Q)keytool -genkeypair -v -keystore $@ -alias debugkey \
	    -storepass android -keypass android \
	    -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Debug"

# +-------------------------------------------------------------------------+ #
# |                  Règles par architecture (template eval)                | #
# +-------------------------------------------------------------------------+ #
define ARCH_TEMPLATE
# --- Compilation du glue (tiers) ---
build/obj/glue-$(1).o: $(GLUE)/android_native_app_glue.c | build/obj
	@printf "== [\033[32;1m$(1)\033[0m] Compilation du code glue (tiers) ==\n"
	$(Q)$(CLANG) --target=$(TRIPLE_$(1)) $(CFLAGS_GLUE) $(INCLUDES) -c $$< -o $$@

# --- Objets utilisateur ---
USER_OBJS_$(1) := $$(patsubst src/%.c,build/obj/%-$(1).o,$$(SRCS))

$$(USER_OBJS_$(1)): build/obj/%-$(1).o: src/%.c | build/obj
	@printf "== [\033[32;1m$(1)\033[0m] Compilation de $$< ==\n"
	$(Q)mkdir -p $$(@D)
	$(Q)$(CLANG) --target=$(TRIPLE_$(1)) $(CFLAGS_USER) $(INCLUDES) -c $$< -o $$@

# --- Édition de liens ---
lib/$(1)/libmain.so: build/obj/glue-$(1).o $$(USER_OBJS_$(1))
	@printf "== [\033[32;1m$(1)\033[0m] Édition de liens ==\n"
	$(Q)mkdir -p $$(@D)
	$(Q)$(CLANG) --target=$(TRIPLE_$(1)) $(LDFLAGS) $$^ -o $$@

# --- Packaging du manifeste + ajout de la lib native ---
build/app-$(1).unsigned.apk: lib/$(1)/libmain.so AndroidManifest.xml | build
	@printf "== [\033[32;1m$(1)\033[0m] Packaging du manifeste ==\n"
	$(Q)aapt package -f -M AndroidManifest.xml -S res -I $(ANDROID_JAR) -F $$@
	@printf "== [\033[32;1m$(1)\033[0m] Ajout de la lib native ==\n"
	$(Q)aapt add $$@ lib/$(1)/libmain.so >/dev/null

# --- Zipalign ---
build/app-$(1).aligned.apk: build/app-$(1).unsigned.apk
	@printf "== [\033[32;1m$(1)\033[0m] Zipalign ==\n"
	$(Q)zipalign -f 4 $$< $$@

# --- Signature ---
build/app-$(1).apk: build/app-$(1).aligned.apk debug.keystore
	@printf "== [\033[32;1m$(1)\033[0m] Signature ==\n"
	$(Q)apksigner sign --ks debug.keystore --ks-pass pass:android --out $$@ $$<

# --- Copie vers storage ---
.PHONY: copy-$(1)
copy-$(1): build/app-$(1).apk
	@if [ -d $(HOME)/storage/downloads ]; then \
	    printf "== [\033[32;1m$(1)\033[0m] Copie vers storage ==\n"; \
	    cp $$< $(HOME)/storage/downloads/domino-$(1).apk; \
	fi
endef

$(foreach a,$(ARCHS),$(eval $(call ARCH_TEMPLATE,$(a))))