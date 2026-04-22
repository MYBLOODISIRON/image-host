DROP DATABASE IF EXISTS `image_host`;
CREATE DATABASE `image_host`;
USE DATABASE `image_host`;


DROP TABLE IF EXISTS `user_info`;
CREATE TABLE `user_info`
(
    `id`        bigint(20)      NOT NULL    PRIMARY KEY     AUTO_INCREMENT,
    `user_name` varchar(32)     NOT NULL    DEFAULT '',
    `nick_name` varchar(32)     NOT NULL    DEFAULT '',
    `password`  varchar(32)     NOT NULL    DEFAULT '',
    `phone`     varchar(16)     NOT NULL    DEFAULT '',
    `email`     varchar(64)     DEFAULT ''.
    `create_time`   timestamp   NULL        DEFAULT CURRENT_TIMESTAMP,

    UNIQUE KEY      `uq_nick_name`      (`nick_name`),
    UNIQUE KEY      `uq_user_name`      (`user_name`)
)ENGINE = InnoDB    AUTO_INCREMENT = 14     DEFAULT CHARSET = utf8 COMMENT = '用户信息表';


DROP TABLE IF EXISTS `user_file_list`;
CREATE TABLE `user_file_list`
(
    `id`        int(11)         NOT NULL        PRIMARY KEY     AUTO_INCREMENT,
    `user`      varchar(32)     NOT NULL,
    `md5`       varchar(256)    NOT NULL,
    `create_time`   timestamp   NULL            DEFAULT CURRENT_TIMESTAMP,
    `file_name`     varchar(128)DEFAULT NULL,
    `share_status`  int(11)     DEFAULT NULL        COMMENT '共享状态，1共享，0未共享',
    `pv`            int(11)     DEFAULT NULL        COMMENT '文件下载量，下载一次加1',

    UNIQUE KEY   `idx_user_md5_file_name`        (`user`, `md5`, `file_name`),
)ENGINE = InnoDb    AUTO_INCREMENT = 30     DEFAULT CHARSET = utf8  COMMENT = '用户文件表';


DROP TABLE IF EXISTS `file_info`;
CREATE TABLE `file_info`
(
    `id`        bigint(20)      NOT NULL    PRIMARY KEY     AUTO_INCREMENT  COMMENT '文件序号，主键自增',
    `md5`       varchar(256)    NOT NULL    COMMENT '文件md5',
    `file_id`   varchar(256)    NOT NULL    COMMENT '文件id:/group1/M00/00/00/xxx.png',
    `url`       varchar(512)    NOT NULL    COMMENT '文件url 192.168.52.139:80/group1/M00/00/00/xxx.png',
    `size`      bigint(20)      DEFAULT '0' COMMENT '文件大小, 以字节为单位',
    `type`      varchar(32)     DEFAULT ''  COMMENT '文件类型： png, zip, mp4……',
    `count`     int(11)         DEFAULT '0' COMMENT '文件引用计数,默认为1。每增加一个用户拥有此文件，此计数器+1',

    UNIQUE KEY  `uq_md5`    (`md5`)
)ENGINE = InnoDB    AUTO_INCREMENT = 70    DEFAULT CHARSET = utf8       COMMENT = '文件信息表';


DROP TABLE IF EXISTS `share_file_list`;
CREATE TABLE `share_file_list`
(
  `id`      int(11)         NOT NULL    AUTO_INCREMENT  COMMENT '编号',
  `user`    varchar(32)     NOT NULL    COMMENT '文件所属用户',
  `md5`     varchar(256)    NOT NULL    COMMENT '文件md5',
  `file_name`   varchar(128)    DEFAULT NULL    COMMENT '文件名字',
  `pv`          int(11)         DEFAULT '1'     COMMENT '文件下载量，默认值为1，下载一次加1',
  `create_time` timestamp       NULL    DEFAULT CURRENT_TIMESTAMP       COMMENT '文件共享时间',

  PRIMARY KEY   (`id`),
  key   `idx_filename_md5_user`     (`file_name`, `md5`, `user`),
  key   `idx_md5_user`              (`md5`, `user`)
)ENGINE = InnoDB   AUTO_INCREMENT = 16     DEFAULT CHARSET=utf8    COMMENT='共享文件列表';



DROP TABLE IF EXISTS `share_picture_list`;
CREATE TABLE `share_picture_list`
(
  `id`      int(11)         NOT NULL    AUTO_INCREMENT  COMMENT '编号',
  `user`    varchar(32)     NOT NULL    COMMENT '文件所属用户',
  `filemd5` varchar(256)    NOT NULL    COMMENT '文件md5',
  `file_name` varchar(128)  DEFAULT NULL    COMMENT '文件名字',
  `urlmd5`      varchar(256)    NOT NULL    COMMENT '图床urlmd5',
  `key`         varchar(8)      NOT NULL    COMMENT '提取码',
  `pv`          int(11)         DEFAULT '1' COMMENT '文件下载量，默认值为1，下载一次加1',
  `create_time` timestamp       NULL        DEFAULT CURRENT_TIMESTAMP   COMMENT '文件创建时间',

  PRIMARY KEY   (`id`),
  KEY   `idx_user_filemd5`  (`user`, `filemd5`), 
  KEY   `idx_urlmd5_user`   (`urlmd5`, `user`) 
) ENGINE = InnoDB   AUTO_INCREMENT = 16     DEFAULT CHARSET=utf8    COMMENT='图床文件列表';