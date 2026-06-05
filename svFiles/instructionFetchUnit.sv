module instructionFetchUnit (
    input  logic        clk,
    input  logic        rst_n,
    input  logic        next_instr_en, 
    output logic [31:0] current_instr, 
    output logic        stream_end     
);

    logic [31:0] instr_rom [0:255];
    logic [7:0]  pc;

    initial begin
        $readmemh("instr_rom.txt", instr_rom);
    end

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pc <= 8'd0;
        end else if (next_instr_en && !stream_end) begin
            pc <= pc + 1; 
        end
    end

    assign current_instr = instr_rom[pc];
    assign stream_end    = (instr_rom[pc] == 32'd0); 

endmodule