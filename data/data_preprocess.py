# -*- coding:utf-8 -*-
#!/usr/bin/env python3
#
#   Author  :   Dounm
#   E-mail  :   niuchong893184@gmail.com
#   Date    :   17/02/07 16:31:38
#   Desc    :   count words freq and substitute word by integer


import os
import sys

def save_wordids(wordid_list, k = 10):
    # split wordid_list into k parts
    wordid_list = map(lambda x: str(x), wordid_list)
    word_num = len(wordid_list)
    k = min(k, word_num)
    aver_num = word_num // k

    file_content_list = [None] * k
    for i in range(k):
        begin = i * aver_num
        if i == k-1:
            end = word_num
        else:
            end = (i + 1) * aver_num
        file_content_list[i] = wordid_list[begin : end]
    
    # write into k files   
    if not os.path.isdir('train_data'):
        os.mkdir('train_data')
    
    for i in range(k):
        with open('train_data/text8.p' + str(i), 'w') as f:
            f.write(' '.join(file_content_list[i]))


if __name__ == '__main__': 
    with open('text8', 'r') as f:
        words = f.read().strip().split(' ')

    word_cnt = 0
    word_dict = dict()
    wordids = []

    for word in words:
        if word in word_dict:
            word_dict[word][1] += 1
        else:
            wordids.append(word_cnt)
            word_dict[word] = [word_cnt, 1]
            word_cnt += 1

    assert(word_cnt == len(word_dict))
    with open('text8.dict', 'w') as f:
        f.write(str(word_cnt) + '\n')
        for word, pair in word_dict.items():
            wordid = str(pair[0])
            word_freq = str(pair[1])
            f.write(word + '\t' + wordid + '\t' + word_freq + '\n')
    
    save_wordids(wordids)
