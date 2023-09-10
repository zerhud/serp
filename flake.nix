{
  description = "cpp universal serialization";
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs { inherit system; };
      der = pkgs.gcc13Stdenv.mkDerivation {
        name = "serp";
        buildInputs = [pkgs.boost];
        nativeBuildInputs = [pkgs.clang_15];
        #installPhase = "mkdir -p \"$out/include\" && cp ascip.hpp -t \"$out/include\" && cp -rt \"$out/include\" ascip";
        #buildPhase = "g++ -std=c++23 -fwhole-program -march=native ./test.cpp -o ascip_test && ./ascip_test";
        meta.description = "cpp universal serialization library.";
        src = ./.;
      };
    in rec {
      devShell = der;
      packages.default = der;
      packages.serp = der;
      defaultPackage = der;
    });
}
