namespace "test" do
    file :cpdbdata do
        sh "git clone git://github.com/amaunz/cpdbdata.git"
    end

    task :loo => ["cpdbdata"] do
        sh "make lazar"
        `mkdir -p test`

        commits = `git log -2 | grep commit | sed 's/^.* //g'`.split(' ')
        cur_commit = commits[0]
        prev_commit = commits[1]
    
        puts "CC: #{cur_commit}"
        puts "PC: #{prev_commit}"

        cmd_cur = "./lazar -s cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.smi -t cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.class -f cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.fminer.f6.l2.a.linfrag -x > test/#{cur_commit}.loo"
        cmd_prev = "./lazar -s cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.smi -t cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.class -f cpdbdata/salmonella_mutagenicity/salmonella_mutagenicity_alt.fminer.f6.l2.a.linfrag -x > test/#{prev_commit}.loo"

        if File.exists?("test/#{prev_commit}.loo")
            if !File.exists?("test/#{cur_commit}.loo")
                sh cmd_cur
            else
                puts "test.loo: File '#{cur_commit}.loo' exists."
            end
            dif = `diff test/#{cur_commit}.loo test/#{prev_commit}.loo`.chomp
            if dif.length > 0
                f= File.new("test/diff_#{cur_commit}_#{prev_commit}",  "w+")
                f.puts(dif)
                f.close
                puts "Difference found between '#{cur_commit}.loo' and '#{prev_commit}.loo'. See file 'test/diff_#{cur_commit}_#{prev_commit}'"
            else
                puts "No difference found between '#{cur_commit}.loo' and '#{prev_commit}.loo' :-)"
            end
        else
            puts "File '#{prev_commit}.loo' does not exist. This means no previous commit was found."
            puts "Initialising test directory!"
            sh cmd_prev
        end
    end
end
