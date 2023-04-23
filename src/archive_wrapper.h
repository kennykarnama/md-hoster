#include<stdio.h>
#include<archive.h>
#include<archive_entry.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>

int gzip(int input_c, const char** input_files, const char* output_file);
int extract_archive(const char* input_file);
int copy_data(struct archive* arc_reader, struct archive* arc_writer);


int gzip(int input_c, const char** input_files, const char* output_file) {
    
    struct archive *arc_writer = archive_write_new();
    archive_write_add_filter(arc_writer, ARCHIVE_FILTER_GZIP);
    archive_write_set_format_pax_restricted(arc_writer);

    archive_write_open_filename(arc_writer, output_file);

    struct archive_entry *entry = NULL;
    struct stat st;
    char buf[8192];

    int ret;

    for (int f = 0; f < input_c; f++) {
        printf("processing file: %s\n", input_files[f]);
        ret = stat(input_files[f], &st);
        if (ret != 0) {
            fprintf(stderr, "stat failed. err: %s\n", strerror(errno));
            return ret;
        }
        entry = archive_entry_new();
        archive_entry_copy_stat(entry, &st);
        archive_entry_set_pathname(entry, input_files[f]);
        archive_write_header(arc_writer, entry);

        int fd = open(input_files[f], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "open failed. file: %s %s\n", input_files[f], strerror(errno));
            return fd;
        }
        
        size_t n_read = read(fd, buf, sizeof(buf));

        while (n_read > 0) {
            archive_write_data(arc_writer, buf, n_read);
            n_read = read(fd, buf, sizeof(buf));
        }

        close(fd);
        archive_entry_free(entry);
    }

    archive_write_close(arc_writer);
    archive_write_free(arc_writer);

    return 0;
}

// extract_archive accepts an input_file
// extract the contents as is
int extract_archive(const char* input_file) {
    int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

    struct archive *arc_reader = archive_read_new();
    archive_read_support_format_all(arc_reader);
    archive_read_support_filter_all(arc_reader);

    struct archive *arc_writer = archive_write_disk_new();
    archive_write_disk_set_options(arc_writer, flags);
    archive_write_disk_set_standard_lookup(arc_writer);

    int ret = archive_read_open_filename(arc_reader, input_file, 10240);
    if (ret != 0) {
        fprintf(stderr, "failed opening file. err: %s\n", archive_error_string(arc_reader));
        return ret;
    }

    struct archive_entry *entry;
    for (;;) {
        ret = archive_read_next_header(arc_reader, &entry);
        if (ret == ARCHIVE_EOF) {
            break;
        }
        if (ret < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(arc_reader));
        }
        if (ret < ARCHIVE_WARN){
            return ret;
        }
        ret = archive_write_header(arc_writer, entry);
        if (ret < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(arc_writer));
        }else if (archive_entry_size(entry) > 0) {
            ret = copy_data(arc_reader, arc_writer);
            if (ret < ARCHIVE_OK) {
                return ret;
            }
        }
    }

    archive_read_close(arc_reader);
    archive_read_free(arc_reader);
    archive_write_close(arc_writer);
    archive_write_free(arc_writer);

    return 0;
}

int copy_data(struct archive* arc_reader, struct archive* arc_writer) {
    int ret;
    const void *buff;
    size_t size;
    la_int64_t offset;
    for (;;) {
        ret = archive_read_data_block(arc_reader, &buff, &size, &offset);
        if (ret == ARCHIVE_EOF) {
            return (ARCHIVE_OK);
        }
        if (ret < ARCHIVE_OK) {
            return ret;
        }
        ret = archive_write_data_block(arc_writer, buff, size, offset);
        if (ret < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(arc_writer));
            return ret;
        }
    }
}