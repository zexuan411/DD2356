{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
  }: {
    devShells.x86_64-linux.default = let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in
      pkgs.mkShell {
        packages = with pkgs; [
          (python3.withPackages (python-pkgs: [
            python-pkgs.numpy
            python-pkgs.matplotlib
            python-pkgs.toolz
            python-pkgs.pystache
          ]))
          black
          llvmPackages.openmp
          mpi
          clang-tools
        ];
      };
  };
}
