#!/usr/bin/env ruby

class Trie
  def initialize depth=0
    @subs = [nil] * 26
    @all_ids = []
    @depth = depth
  end
  
  def insert word, id, idx
    if idx >= word.length
      @all_ids << id
    else
      chr = word[idx].ord - 'a'.ord
      @subs[chr] = Trie.new(idx + 1) unless @subs[chr]
      @subs[chr].insert(word, id, idx + 1)
    end
  end

  def serialize file
    idx_table = [(1 << 32) - 1] * 26
    @subs.each_with_index do |trie,i|
      next unless trie
      idx_table[i] = trie.serialize file
    end
    this_pos = file.tell
    file.write "#{[@depth].pack('l<')}#{idx_table.pack('l<*')}#{[@all_ids.length].pack('l<')}#{@all_ids.pack('l<*')}"
    this_pos
  end
end

def process_file path, full_id, id_table, trie
  puts full_id
  small_id = id_table.length
  id_table << full_id
  File.read(path).split(/\W+/).each do |word|
    word.downcase!
    word.gsub! /[^a-z]+/, ""
    next if word.empty?
    trie.insert word, small_id, 0
  end
end

def rec_fill_table path, id, id_table, trie
  Dir.foreach(path) do |elem|
    next if elem =~ /^\./
    new_path = "#{path}/#{elem}"
    new_id = "#{id}/#{elem}"
    if File.directory? new_path
      rec_fill_table new_path, new_id, id_table, trie
    else
      process_file new_path, new_id, id_table, trie
    end
  end
end

id_table = []
trie = Trie.new
ROOT_DIR = "maildir"

rec_fill_table(ROOT_DIR, "", id_table, trie)

File.open("full_ids", "w") do |f|
  f.write id_table.join("\n")
end

File.open("trie", "w") do |f|
  f.write([trie.serialize(f)].pack("l<"))
end
