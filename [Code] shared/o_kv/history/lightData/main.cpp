#include "cfg1.is.qcr.h"

int main(int argc, char *argv[]) {
	// 设置中文环境，确保输出中文字符正常显示
    setlocale(LC_ALL, "zh_CN.UTF-8");
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	// 创建系统占用，没有文件则会自动创建文件
    FileLock* lock = file_lock_create("test.txt");
    file_lock(lock);

    // 打开文件，注意此处类型为 char* 指针
    char* content = file_read_content(lock);

    // 创建配置的缓存ORM，方便后续进行类DML操作
    Config config;
    config_init(&config);
    config_parse(&config, content);
    free(content);
	file_unlock(lock);

    // 简单地获取一个配置
    const wchar_t* value = wchart_config_get_value(&config, L"说明");
	wprintf(L"获取“说明”的配置 >> %ls\n",value);
	free((void*)value);
	printf("\n");

	// 打印全部配置（需要注意字符类型）
	wprintf(L"打印全部配置:\n");
    for(size_t i = 0; i < config.count; i++) {
		wchar_t* key = narrow_to_wide(config.pairs[i].key);
		wchar_t* value = narrow_to_wide(config.pairs[i].value);
        wprintf(L"配置项 %zu: %ls = %ls\n", i, key, value);
	}
	printf("\n");

	// 向配置ORM中添加新的键值对
	const char* new_key = "website";
	const char* new_value = "https://maichc.club";
	const char* new_key_cn = wide_to_narrow(L"网站地址");
	config_add_pair(&config, new_key, new_value);
	config_add_pair(&config, new_key_cn, new_value);
	wprintf(L"已添加中文键值对和英文键值对");
	printf("\n");

	// 查询 ORM 新增的配置项
    wprintf(L"索引新的键值对:\n");
    wprintf(L"网站地址 = %ls\n",wchart_config_get_value(&config,L"网站地址"));
	wprintf(L"website = %ls\n", wchart_config_get_value(&config, L"website"));
    wprintf(L"[其他获取方式] website = %ls\n", narrow_to_wide(config_get_value(&config,"website")));
	printf("\n");


	// 更新配置项
	wprintf(L"更新配置项 “website”\n");
	config_update_pair(&config, "website", "https://github.com/zomaii");
	wprintf(L"[其他获取方式] website = %ls\n", narrow_to_wide(config_get_value(&config, "website")));
	printf("\n");
	
	// 删除配置项
	wprintf(L"删除配置项 “website”\n");
	config_delete_pair(&config, "website");
	wprintf(L"删除后全部配置:\n");
	for (size_t i = 0; i < config.count; i++) {
		wchar_t* key = narrow_to_wide(config.pairs[i].key);
		wchar_t* value = narrow_to_wide(config.pairs[i].value);
		wprintf(L"配置项 %zu: %ls = %ls\n", i, key, value);
	}
	printf("\n");



	// 提交配置的 ORM 数据到文件,这里另外存为一个文件
	wprintf(L"提交配置到文件...\n");
	FileLock* toFile = file_lock_create("config.qcr");
	file_write_content(toFile,config_serialize(&config));

	config_free(&config);
	file_unlock(toFile);

    return 0;
}