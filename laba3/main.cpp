#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

std::string calculate_sha1(const fs::path& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		throw std::runtime_error("Не удалось открыть файл: " + filepath.string());
	}

	SHA_CTX ctx;
	SHA1_Init(&ctx);
	std::vector<char> buffer(4096);

	while (file) {
		file.read(buffer.data(), buffer.size());
		std::streamsize bytes = file.gcount();
		if (bytes > 0) {
			SHA1_Update(&ctx, buffer.data(), bytes);
		}
	}

	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1_Final(hash, &ctx);

	std::ostringstream oss;
	for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
		oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
	}
	return oss.str();
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Использование: " << argv[0] << " <путь_к_каталогу>\n";
		return 1;
	}

	fs::path root_dir = argv[1];
	if (!fs::exists(root_dir) || !fs::is_directory(root_dir)) {
		std::cerr << "Ошибка: Указанный путь не является каталогом.\n";
		return 1;
	}

	std::unordered_map<std::string, fs::path> hash_map;

	for (const auto& entry : fs::recursive_directory_iterator(root_dir)) {
		if (!entry.is_regular_file()) {
			continue;
		}

		fs::path current_path = entry.path();

		try {
			std::string hash = calculate_sha1(current_path);

			if (hash_map.find(hash) != hash_map.end()) {
				fs::path original_path = hash_map[hash];

				if (fs::equivalent(original_path, current_path)) {
					continue;
				}

				struct stat st_orig, st_curr;
				stat(original_path.c_str(), &st_orig);
				stat(current_path.c_str(), &st_curr);

				if (st_orig.st_dev != st_curr.st_dev) {
					std::cerr << "Пропуск: нельзя создать жёсткую ссылку между разными ФС: " << current_path << "\n";
					continue;
				}

				std::cout << "Дубликат найден:\n" << " Оригинал: " << original_path << "\n" << " Ссылка: " << current_path << "\n";

				fs::remove(current_path);
				fs::create_hard_link(original_path, current_path);
				std::cout << " -> Заменено на жесткую ссылку.\n";
			} else {
				hash_map[hash] = current_path;
			}
		} catch (const std::exception& e) {
			std::cerr << "Ошибка при обработке " << current_path << ": " << e.what() << "\n";
		}
	}

	std::cout << "Обработка завершена.\n";
	return 0;
}
