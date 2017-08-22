string arrange(string sentence) {
    string temp = "The lines are printed in reverse order.";
    string gExp = "^[A-Z][a-z ]*\.$";
    
    if ((sentence.size() < 1) || (sentence.size() > 100000)) {
        return "";
    }
    
    string s = temp;
    string ret;
    ret.clear();
    vector<int>     posSpace;
    vector<string>  words;
    vector<string>  mark;
    words.clear();
    mark.clear();
    
    int i = 0;
    int j = 0;
    
    int gExpSpacePos = 0;
    bool check = true;
    
    
    // check general express
    while(check) {
        gExpSpacePos = s.find(gExp);
        if (gExpSpacePos != -1) {       
            s.erase(gExpSpacePos, gExp.length());
            s.insert(gExpSpacePos, " ");
        }
        else {
            check = false;
        }
                                                                                                            }
    
    
    int32 idx = 0;
    int32 markIdx = 0;
    string t;
    string tMark;
    t.clear();
    tMark.clear();
    bool findS = false;
    for (i = 0; i < s.size(); i++) {
        if (s[i] == ' ') {
            posSpace.push_back(i);
            findS = true;
        }
        else if ((s[i] >= 'A' && s[i] <= 'Z') ||
                (s[i] >= 'a' && s[i] <= 'z')) {
            t += s[i];
        }
        else {
            tMark += s[i];
            mark.push_back(tMark);
            markIdx++;
            findS = true;
        }
        
        
        if (findS == true) {
            if (t != "") {
                words.push_back(t);
                t.clear();
            }
            findS = false;
        }
    }
    
#if 0
    vector<int> slength;
    for (i = 0; i < words.size(); i++) {
        slength.push_back(words[i].length());
    }
#endif
    
    check = true;
    bool check2 = false;
    bool check3 = false;
    i = 0;
    string tt;
    while(check) {
#if 0
        if (i < words.size() - 1) {
            if (words[i].size() > words[i + 1].size()) {
                tt.clear();
                tt = words[i];
                words[i] = words[i + 1];
                words[i + 1] = tt;
            }
            i++;
        }
#endif
        
        if (!check2) {
            for (i = 0; i < words.size() - 1; i++) {
                if (words[i].size() > words[i + 1].size()) {
                    tt.clear();
                    tt = words[i];
                    words[i] = words[i + 1];
                    words[i + 1] = tt;
                    tt.clear();
                    check2 = true;
                }
            }
        }
        
        
        if (check2) {
            check2 = false;
        }
        else {
            check = false;
        }
    }
    
    int diff = 'a' - 'A';
    
    for (i = 0; i < words.size(); i++) {
        if (i == 0) {
            tt.clear();
            tt = words[i];
            if (tt[0] >= 'a' && tt[0] <= 'z') {
                tt[0] -= diff;
                words[i] = tt;
            }
        }
        else {
            tt.clear();
            tt = words[i];
            if (tt[0] >= 'A' && tt[0] <= 'Z') {
                tt[0] += diff;
                words[i] = tt;
            }
        }
        ret += words[i];
        if (i != words.size() - 1) {
            ret += " ";
        }
    }
    
    for (i = 0; i < mark.size(); i++) {
        ret += mark[i];
        if (i != mark.size() - 1) {
            ret += " ";
        }
    }
    
    return ret;
}
